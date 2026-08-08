#!/usr/bin/env python

# Copyright 2026 NXP
#
# SPDX-License-Identifier: BSD-3-Clause

"""Packet logging helpers for eRPC framed transports."""

import atexit
import binascii
import datetime
import importlib
import inspect
import json
import os
import pprint
import sys
import threading
import time

from .codec import MessageType


_PACKET_LOG_ENV = "ERPC_PYTHON_PACKET_LOG"
_PACKET_LOG_FORMAT_ENV = "ERPC_PYTHON_PACKET_LOG_FORMAT"
_PACKET_LOG_DECODE_ENV = "ERPC_PYTHON_PACKET_LOG_DECODE"
_PACKET_LOG_APPEND_ENV = "ERPC_PYTHON_PACKET_LOG_APPEND"
_PACKET_LOG_MAX_BYTES_ENV = "ERPC_PYTHON_PACKET_LOG_MAX_BYTES"
_PACKET_LOG_IDL_ENV = "ERPC_PYTHON_PACKET_LOG_IDL"
_PACKET_LOG_PRETTY_ENV = "ERPC_PYTHON_PACKET_LOG_PRETTY"
_PACKET_LOG_ENV_NAMES = (
    _PACKET_LOG_ENV,
    _PACKET_LOG_FORMAT_ENV,
    _PACKET_LOG_DECODE_ENV,
    _PACKET_LOG_APPEND_ENV,
    _PACKET_LOG_MAX_BYTES_ENV,
    _PACKET_LOG_IDL_ENV,
    _PACKET_LOG_PRETTY_ENV,
)
_FALSE_VALUES = ("0", "false", "no", "off", "disabled", "none")
_TRUE_VALUES = ("1", "true", "yes", "on", "enabled")
_packet_logger_from_env = None
_packet_logger_env_key = None


class PacketLogRecord(object):
    """eRPC framed transport packet log record.

    Args:
        direction (str): Packet direction, either ``"TX"`` or ``"RX"``.
        transport (object): Transport instance that produced the packet.
        frame (bytes or bytearray): Complete framed transport bytes.
        header (bytes or bytearray): Framed transport header bytes.
        payload (bytes or bytearray): eRPC message payload bytes.
        header_crc (int): Header CRC value from the frame.
        message_length (int): eRPC message payload length from the frame.
        body_crc (int): Body CRC value from the frame.
        computed_header_crc (int): Locally computed header CRC.
        computed_body_crc (int): Locally computed body CRC.
        header_crc_ok (bool): True when the header CRC matches.
        body_crc_ok (bool): True when the body CRC matches.
        error (str): Optional error description.

    The transport instance ID is included in decoded records so generated IDL
    reply matching can distinguish packets from different transport objects.
    """

    SCHEMA_VERSION = 1

    def __init__(self, direction, transport, frame, header, payload, header_crc=None, message_length=None,
                 body_crc=None, computed_header_crc=None, computed_body_crc=None, header_crc_ok=None,
                 body_crc_ok=None, error=None):
        self.direction = direction
        self.timestamp, self.timestamp_ns = _timestamp_info()
        self.transport = transport.__class__.__name__ if transport is not None else None
        self.transport_id = id(transport) if transport is not None else None
        self.frame = _to_bytes(frame)
        self.header = _to_bytes(header)
        self.payload = _to_bytes(payload)
        self.header_crc = header_crc
        self.message_length = message_length
        self.body_crc = body_crc
        self.computed_header_crc = computed_header_crc
        self.computed_body_crc = computed_body_crc
        self.header_crc_ok = header_crc_ok
        self.body_crc_ok = body_crc_ok
        self.error = error

    def to_dict(self, decode=True, idl_decoder=None):
        """Return this packet log record as a JSON-serializable dictionary.

        Args:
            decode (bool): Decode the BasicCodec message header when possible.
            idl_decoder (GeneratedPythonIdlDecoder): Optional generated Python IDL decoder.

        Returns:
            dict: JSON-serializable packet log record.
        """
        result = {
            "version": self.SCHEMA_VERSION,
            "time": self.timestamp,
            "time_ns": self.timestamp_ns,
            "direction": self.direction,
            "transport": self.transport,
            "transport_id": self.transport_id,
            "frame_len": len(self.frame),
            "header_hex": _hex(self.header),
            "payload_hex": _hex(self.payload),
            "frame_hex": _hex(self.frame),
            "framed": {
                "header_crc": self.header_crc,
                "message_length": self.message_length,
                "body_crc": self.body_crc,
                "computed_header_crc": self.computed_header_crc,
                "computed_body_crc": self.computed_body_crc,
                "header_crc_ok": self.header_crc_ok,
                "body_crc_ok": self.body_crc_ok,
            },
            "error": self.error,
        }

        if decode:
            result["message"] = decode_basic_message(self.payload, idl_decoder=idl_decoder,
                                                     transport_id=self.transport_id)

        return result


class PacketLogger(object):
    """Base class for eRPC packet loggers."""

    def log_packet(self, record):
        """Log an eRPC packet record.

        Args:
            record (PacketLogRecord): Packet record to log.
        """
        raise NotImplementedError()

    def log_tx(self, record):
        """Log a TX packet record.

        Args:
            record (PacketLogRecord): Packet record to log.
        """
        self.log_packet(record)

    def log_rx(self, record):
        """Log an RX packet record.

        Args:
            record (PacketLogRecord): Packet record to log.
        """
        self.log_packet(record)


class ConsolePacketLogger(PacketLogger):
    """Packet logger that writes human-readable packet logs to a stream.

    Args:
        stream (object): Text stream to write to. Defaults to ``sys.stdout``.
        decode (bool): Decode the BasicCodec message header when possible.
        max_bytes (int): Optional maximum number of frame bytes printed in text output.
        close_stream (bool): Close the stream when this logger is closed.
        idl_decoder (GeneratedPythonIdlDecoder): Optional generated Python IDL decoder.
    """

    def __init__(self, stream=None, decode=True, max_bytes=None, close_stream=False, idl_decoder=None):
        self._stream = stream if stream is not None else sys.stdout
        self._decode = decode
        self._max_bytes = max_bytes
        self._close_stream = close_stream
        self._idl_decoder = idl_decoder
        self._lock = threading.Lock()

    def close(self):
        """Close the output stream when this logger owns it."""
        if self._close_stream:
            self._stream.close()

    def log_packet(self, record):
        """Write a human-readable packet log entry.

        Args:
            record (PacketLogRecord): Packet record to log.
        """
        data = record.to_dict(decode=self._decode, idl_decoder=self._idl_decoder)
        framed = data["framed"]
        line = "%s %s %s frame_len=%d header_crc=%s body_crc=%s error=%s" % (
            data["time"],
            data["direction"],
            data["transport"],
            data["frame_len"],
            _format_optional_hex(framed["header_crc"]),
            _format_optional_hex(framed["body_crc"]),
            data["error"],
        )
        with self._lock:
            self._stream.write(line + "\n")

            if self._decode and data.get("message") is not None:
                self._stream.write("  message: %s\n" % _format_message(data["message"]))
                decoded = _get_decoded_payload(data["message"])
                if decoded is not None:
                    self._stream.write("  decoded: %s\n" % pprint.pformat(decoded, width=120))

            frame = record.frame
            suffix = ""
            if self._max_bytes is not None and len(frame) > self._max_bytes:
                frame = frame[:self._max_bytes]
                suffix = " ..."
            self._stream.write("  frame: %s%s\n" % (_hex_spaced(frame), suffix))
            self._stream.flush()


class JsonLinesPacketLogger(PacketLogger):
    """Packet logger that writes JSON records for framed packets.

    The default output is strict JSON Lines: one compact JSON object per line.
    Set ``pretty`` to true for human-readable multi-line JSON records.

    Args:
        filename (str): Output JSON Lines file name.
        decode (bool): Decode the BasicCodec message header when possible.
        append (bool): Append to an existing file when true. Overwrite when false.
        pretty (bool): Pretty-print JSON records for manual inspection.
        idl_decoder (GeneratedPythonIdlDecoder): Optional generated Python IDL decoder.
    """

    def __init__(self, filename, decode=True, append=True, pretty=False, idl_decoder=None):
        self._decode = decode
        self._pretty = pretty
        self._idl_decoder = idl_decoder
        self._lock = threading.Lock()
        mode = "a" if append else "w"
        self._file = open(filename, mode)

    def close(self):
        """Close the JSON Lines output file."""
        self._file.close()

    def log_packet(self, record):
        """Write a JSON packet log entry.

        Args:
            record (PacketLogRecord): Packet record to log.
        """
        with self._lock:
            if self._pretty:
                json.dump(record.to_dict(decode=self._decode, idl_decoder=self._idl_decoder), self._file, indent=2,
                          sort_keys=True)
                self._file.write("\n")
            else:
                json.dump(record.to_dict(decode=self._decode, idl_decoder=self._idl_decoder), self._file,
                          sort_keys=True)
                self._file.write("\n")
            self._file.flush()

    def __enter__(self):
        return self

    def __exit__(self, exc_type, exc, exc_tb):
        self.close()
        return False


class GeneratedPythonIdlDecoder(object):
    """Decoder that uses generated Python eRPC modules for IDL-aware logs.

    Args:
        modules (list): Generated Python package or interface module names.

    The decoder can always map service and method IDs to names when generated
    ``interface.py`` modules are available. For generated packages that also
    expose ``server.py`` and ``client.py``, it best-effort decodes invocation
    arguments and matching reply reference outputs.
    """

    def __init__(self):
        self._services = {}
        self._errors = []
        self._request_cache = {}
        self._request_cache_order = []
        self._request_cache_limit = 128

    @classmethod
    def from_modules(cls, modules):
        """Create an IDL decoder from generated Python module names.

        Args:
            modules (list): Module names such as ``service.erpc_matrix_multiply``.

        Returns:
            GeneratedPythonIdlDecoder: Configured decoder instance.
        """
        decoder = cls()
        for module_name in modules:
            decoder.add_module(module_name)
        return decoder

    def add_module(self, module_name):
        """Add a generated Python eRPC package or interface module.

        Args:
            module_name (str): Importable module name.
        """
        module_name = module_name.strip()
        if not module_name:
            return

        try:
            module = importlib.import_module(module_name)
            for binding in _load_idl_bindings(module):
                self._services[binding.service_id] = binding
        except Exception as exc:
            self._errors.append({"module": module_name, "error": str(exc)})

    def decode(self, payload, message, transport_id=None):
        """Decode IDL metadata and payload details for a BasicCodec message.

        Args:
            payload (bytes or bytearray): Complete eRPC message payload.
            message (dict): BasicCodec message dictionary.
            transport_id (int): Optional transport instance ID used to isolate reply matching.

        Returns:
            dict: IDL decode details, or ``None`` when no matching IDL was found.
        """
        if not message or message.get("error"):
            return None

        service_id = message.get("service")
        request_id = message.get("request")
        sequence = message.get("sequence")
        binding = self._services.get(service_id)
        if binding is None:
            if self._errors and not self._services:
                return {"error": "IDL module loading failed", "load_errors": self._errors}
            return None

        method = binding.methods.get(request_id)
        result = {
            "module": binding.module_name,
            "interface": binding.interface_name,
            "service": binding.service_name,
        }
        if method is not None:
            result.update({
                "method": method.method_name,
                "method_id_name": method.method_id_name,
            })
        else:
            result["error"] = "unknown request id %s for service %s" % (request_id, service_id)
            return result

        payload_decode = self._decode_payload(binding, method, payload, message, transport_id)
        if payload_decode is not None:
            result["payload"] = payload_decode

        if message.get("type") in ("kInvocationMessage", "kOnewayMessage") and payload_decode is not None:
            self._store_request(transport_id, service_id, request_id, sequence, method, payload_decode)

        return result

    def _decode_payload(self, binding, method, payload, message, transport_id):
        msg_type = message.get("type")
        if msg_type in ("kInvocationMessage", "kOnewayMessage"):
            return self._decode_invocation(binding, method, payload, message)
        if msg_type == "kReplyMessage":
            return self._decode_reply(binding, method, payload, message, transport_id)
        return None

    def _decode_invocation(self, binding, method, payload, message):
        if binding.server_class is None:
            return {"direction": "request", "error": "generated server module is not available"}

        try:
            from .basic_codec import BasicCodec
        except Exception as exc:
            return {"direction": "request", "error": str(exc)}

        handler = _CaptureHandler()
        try:
            service = binding.server_class(handler)
            handler_fn = getattr(service, "_methods", {}).get(method.request_id)
            if handler_fn is None:
                return {"direction": "request", "error": "generated server handler is not available"}

            codec = BasicCodec()
            codec.buffer = bytearray(payload)
            codec.start_read_message()
            try:
                handler_fn(message.get("sequence"), codec)
            except Exception:
                if handler.args is None:
                    raise

            if handler.args is None:
                return {"direction": "request", "error": "generated handler did not capture arguments"}

            return {
                "direction": "request",
                "arguments": _format_arguments(method.arg_names, handler.args),
                "_raw_args": handler.args,
            }
        except Exception as exc:
            return {"direction": "request", "error": str(exc)}

    def _decode_reply(self, binding, method, payload, message, transport_id):
        cache_key = (transport_id, message.get("service"), message.get("request"), message.get("sequence"))
        cached = self._request_cache.get(cache_key)
        if cached is None:
            return {"direction": "reply", "error": "matching invocation was not decoded by this logger"}
        if binding.client_class is None:
            return {"direction": "reply", "error": "generated client module is not available"}

        raw_args = cached.get("raw_args")
        if raw_args is None:
            return {"direction": "reply", "error": "matching invocation has no raw argument cache"}

        try:
            manager = _ReplayClientManager(payload, message.get("sequence"))
            client = binding.client_class(manager)
            args, refs = _clone_client_args(method.arg_names, raw_args)
            getattr(client, method.method_name)(*args)
            return {
                "direction": "reply",
                "references": _format_references(refs),
            }
        except Exception as exc:
            return {"direction": "reply", "error": str(exc)}

    def _store_request(self, transport_id, service_id, request_id, sequence, method, payload_decode):
        raw_args = payload_decode.pop("_raw_args", None)
        cache_key = (transport_id, service_id, request_id, sequence)
        self._request_cache[cache_key] = {
            "method": method,
            "raw_args": raw_args,
        }
        self._request_cache_order.append(cache_key)
        while len(self._request_cache_order) > self._request_cache_limit:
            old_key = self._request_cache_order.pop(0)
            self._request_cache.pop(old_key, None)


class _IdlServiceBinding(object):
    """Generated Python IDL service binding."""

    def __init__(self, module_name, interface_name, service_name, service_id, methods, server_class=None,
                 client_class=None):
        self.module_name = module_name
        self.interface_name = interface_name
        self.service_name = service_name
        self.service_id = service_id
        self.methods = methods
        self.server_class = server_class
        self.client_class = client_class


class _IdlMethodBinding(object):
    """Generated Python IDL method binding."""

    def __init__(self, request_id, method_id_name, method_name, arg_names):
        self.request_id = request_id
        self.method_id_name = method_id_name
        self.method_name = method_name
        self.arg_names = arg_names


class _CaptureHandler(object):
    """Handler object used to capture generated server method arguments."""

    def __init__(self):
        self.method_name = None
        self.args = None

    def __getattr__(self, name):
        def _capture(*args):
            self.method_name = name
            self.args = args
            return None
        return _capture


class _ReplayClientManager(object):
    """Client manager used to replay generated client reply decoding."""

    def __init__(self, reply_payload, sequence):
        self._reply_payload = bytearray(reply_payload)
        self._sequence = sequence

    def create_request(self, isOneway=False):
        from .basic_codec import BasicCodec

        return _ReplayRequestContext(self._sequence, BasicCodec(), isOneway)

    def perform_request(self, request):
        request.codec.buffer = bytearray(self._reply_payload)
        request.codec.start_read_message()


class _ReplayRequestContext(object):
    """Request context used by _ReplayClientManager."""

    def __init__(self, sequence, codec, is_oneway):
        self._sequence = sequence
        self._codec = codec
        self._is_oneway = is_oneway

    @property
    def sequence(self):
        return self._sequence

    @property
    def codec(self):
        return self._codec

    @property
    def is_oneway(self):
        return self._is_oneway


def decode_basic_message(payload, idl_decoder=None, transport_id=None):
    """Decode the eRPC BasicCodec message header.

    Args:
        payload (bytes or bytearray): eRPC message payload bytes.
        idl_decoder (GeneratedPythonIdlDecoder): Optional generated Python IDL decoder.
        transport_id (int): Optional transport instance ID used by IDL reply matching.

    Returns:
        dict: Decoded message header fields, or an error field when decoding fails.
    """
    import struct

    payload = _to_bytes(payload)
    if len(payload) < 8:
        return {
            "codec": "basic",
            "error": "message too short for BasicCodec header",
        }

    try:
        header, sequence = struct.unpack_from('<II', payload, 0)
        version = (header >> 24) & 0xff
        service = (header >> 16) & 0xff
        request = (header >> 8) & 0xff
        msg_type_value = header & 0xff
        try:
            msg_type = MessageType(msg_type_value).name
        except ValueError:
            msg_type = "unknown(%d)" % msg_type_value

        result = {
            "codec": "basic",
            "version": version,
            "type": msg_type,
            "type_value": msg_type_value,
            "service": service,
            "request": request,
            "sequence": sequence,
        }

        if idl_decoder is not None:
            idl = idl_decoder.decode(payload, result, transport_id=transport_id)
            if idl is not None:
                result["idl"] = idl

        return result
    except Exception as exc:
        return {
            "codec": "basic",
            "error": str(exc),
        }


def get_default_packet_logger(environ=None):
    """Return the default packet logger configured through environment variables.

    The logger is disabled by default. Set ``ERPC_PYTHON_PACKET_LOG`` to
    ``stdout``, ``stderr`` or a file path to enable packet logging without
    changing application source code.

    Args:
        environ (dict): Optional environment mapping. Defaults to ``os.environ``.

    Returns:
        PacketLogger: Configured packet logger, or ``None`` when disabled.
    """
    global _packet_logger_from_env
    global _packet_logger_env_key

    env = environ if environ is not None else os.environ
    key = tuple((name, env.get(name)) for name in _PACKET_LOG_ENV_NAMES)

    if environ is None and key == _packet_logger_env_key:
        return _packet_logger_from_env

    logger = create_packet_logger_from_env(env)

    if environ is None:
        if _packet_logger_from_env is not logger:
            _close_packet_logger(_packet_logger_from_env)
        _packet_logger_from_env = logger
        _packet_logger_env_key = key

    return logger


def create_packet_logger_from_env(environ=None):
    """Create a packet logger from environment variables.

    Environment variables:
        ERPC_PYTHON_PACKET_LOG: ``stdout``, ``stderr`` or output file path.
        ERPC_PYTHON_PACKET_LOG_FORMAT: ``text`` or ``jsonl``. Defaults to
            ``text`` for console output and ``jsonl`` for file output.
        ERPC_PYTHON_PACKET_LOG_DECODE: Enable BasicCodec header decode. Defaults
            to enabled.
        ERPC_PYTHON_PACKET_LOG_APPEND: Append to JSONL files. Defaults to
            enabled.
        ERPC_PYTHON_PACKET_LOG_MAX_BYTES: Optional console frame byte limit.
        ERPC_PYTHON_PACKET_LOG_IDL: Comma-separated generated Python IDL modules.
        ERPC_PYTHON_PACKET_LOG_PRETTY: Pretty-print JSON records. Defaults to
            disabled to keep strict JSON Lines output.

    Args:
        environ (dict): Optional environment mapping. Defaults to ``os.environ``.

    Returns:
        PacketLogger: Configured packet logger, or ``None`` when disabled.
    """
    env = environ if environ is not None else os.environ
    destination = env.get(_PACKET_LOG_ENV)
    if destination is None or _is_false(destination):
        return None

    destination = destination.strip()
    if not destination:
        return None

    decode = _get_bool(env, _PACKET_LOG_DECODE_ENV, True)
    append = _get_bool(env, _PACKET_LOG_APPEND_ENV, True)
    pretty = _get_bool(env, _PACKET_LOG_PRETTY_ENV, False)
    max_bytes = _get_int(env, _PACKET_LOG_MAX_BYTES_ENV, None)
    log_format = env.get(_PACKET_LOG_FORMAT_ENV)
    log_format = log_format.strip().lower() if log_format is not None else None
    idl_decoder = create_idl_decoder_from_env(env)

    if destination.lower() in ("stdout", "console"):
        return ConsolePacketLogger(stream=sys.stdout, decode=decode, max_bytes=max_bytes, idl_decoder=idl_decoder)
    if destination.lower() == "stderr":
        return ConsolePacketLogger(stream=sys.stderr, decode=decode, max_bytes=max_bytes, idl_decoder=idl_decoder)

    if log_format is None:
        log_format = "jsonl"

    if log_format == "jsonl":
        return JsonLinesPacketLogger(destination, decode=decode, append=append, pretty=pretty, idl_decoder=idl_decoder)
    if log_format == "text":
        return ConsolePacketLogger(
            stream=open(destination, "a" if append else "w"), decode=decode, max_bytes=max_bytes, close_stream=True,
            idl_decoder=idl_decoder)

    raise ValueError("unsupported ERPC_PYTHON_PACKET_LOG_FORMAT: %s" % log_format)


def create_idl_decoder_from_env(environ=None):
    """Create a generated Python IDL decoder from environment variables.

    Args:
        environ (dict): Optional environment mapping. Defaults to ``os.environ``.

    Returns:
        GeneratedPythonIdlDecoder: Configured decoder, or ``None`` when disabled.
    """
    env = environ if environ is not None else os.environ
    value = env.get(_PACKET_LOG_IDL_ENV)
    if value is None or _is_false(value):
        return None

    modules = _split_env_list(value)
    if not modules:
        return None

    return GeneratedPythonIdlDecoder.from_modules(modules)


def reset_default_packet_logger():
    """Reset the cached environment-configured packet logger.

    This helper is primarily intended for tests and applications that change
    eRPC packet logging environment variables at runtime.
    """
    global _packet_logger_from_env
    global _packet_logger_env_key

    _close_packet_logger(_packet_logger_from_env)
    _packet_logger_from_env = None
    _packet_logger_env_key = None


def _close_default_packet_logger():
    reset_default_packet_logger()


def _load_idl_bindings(module):
    bindings = []
    interface_module = _get_generated_submodule(module, "interface")
    if interface_module is None:
        interface_module = module

    server_module = _get_generated_submodule(module, "server")
    client_module = _get_generated_submodule(module, "client")
    module_name = _get_package_module_name(module, interface_module)

    for name in dir(interface_module):
        interface_class = getattr(interface_module, name)
        if not inspect.isclass(interface_class) or not hasattr(interface_class, "SERVICE_ID"):
            continue

        service_id = getattr(interface_class, "SERVICE_ID")
        if not isinstance(service_id, int):
            continue

        interface_name = interface_class.__name__
        service_name = interface_name[1:] if interface_name.startswith("I") else interface_name
        methods = _load_idl_methods(interface_class)
        server_class = _find_generated_class(server_module, service_name, "Service")
        client_class = _find_generated_class(client_module, service_name, "Client")
        bindings.append(_IdlServiceBinding(
            module_name, interface_name, service_name, service_id, methods, server_class=server_class,
            client_class=client_class))

    return bindings


def _get_generated_submodule(module, name):
    submodule = getattr(module, name, None)
    if submodule is not None:
        return submodule

    module_name = getattr(module, "__name__", "")
    if module_name.endswith("." + name):
        return module

    if hasattr(module, "__path__"):
        try:
            return importlib.import_module(module_name + "." + name)
        except Exception:
            return None

    return None


def _get_package_module_name(module, interface_module):
    module_name = getattr(module, "__name__", None)
    if module_name is not None and not module_name.endswith(".interface"):
        return module_name

    interface_name = getattr(interface_module, "__name__", module_name)
    if interface_name and interface_name.endswith(".interface"):
        return interface_name[:-len(".interface")]
    return interface_name


def _load_idl_methods(interface_class):
    methods = {}
    method_names = []
    for name in dir(interface_class):
        if name.startswith("_"):
            continue
        value = getattr(interface_class, name)
        if callable(value) and not name.isupper():
            method_names.append(name)

    for name in dir(interface_class):
        if not name.endswith("_ID") or name == "SERVICE_ID":
            continue
        request_id = getattr(interface_class, name)
        if not isinstance(request_id, int):
            continue
        method_name = _match_method_name(name[:-3], method_names)
        method = getattr(interface_class, method_name, None)
        methods[request_id] = _IdlMethodBinding(request_id, name, method_name, _get_callable_arg_names(method))

    return methods


def _match_method_name(method_id_prefix, method_names):
    normalized = method_id_prefix.upper()
    for method_name in method_names:
        if method_name.upper() == normalized:
            return method_name
    for method_name in method_names:
        if method_name.upper().replace("_", "") == normalized.replace("_", ""):
            return method_name
    return method_id_prefix.lower()


def _find_generated_class(module, service_name, suffix):
    if module is None:
        return None

    expected = service_name + suffix
    for name in dir(module):
        value = getattr(module, name)
        if inspect.isclass(value) and name == expected:
            return value

    for name in dir(module):
        value = getattr(module, name)
        if inspect.isclass(value) and name.startswith(service_name) and name.endswith(suffix):
            return value

    return None


def _get_callable_arg_names(func):
    if func is None:
        return []

    try:
        args = inspect.getfullargspec(func).args
    except AttributeError:
        try:
            args = inspect.getargspec(func).args
        except Exception:
            return []
    except Exception:
        return []

    if args and args[0] == "self":
        args = args[1:]
    return list(args)


def _format_arguments(arg_names, args):
    formatted = []
    for index, value in enumerate(args):
        item = {
            "index": index,
            "value": _to_json_value(value),
        }
        if index < len(arg_names):
            item["name"] = arg_names[index]
        formatted.append(item)
    return formatted


def _clone_client_args(arg_names, args):
    cloned = []
    refs = []
    for index, value in enumerate(args):
        if _is_reference(value):
            ref = _make_reference(getattr(value, "value", None))
            cloned.append(ref)
            refs.append((index, arg_names[index] if index < len(arg_names) else None, ref))
        else:
            cloned.append(value)
    return cloned, refs


def _format_references(refs):
    formatted = []
    for index, name, ref in refs:
        item = {
            "index": index,
            "value": _to_json_value(getattr(ref, "value", None)),
        }
        if name is not None:
            item["name"] = name
        formatted.append(item)
    return formatted


def _make_reference(value=None):
    try:
        from . import Reference
        return Reference(value)
    except Exception:
        class _Reference(object):
            def __init__(self, ref_value=None):
                self.value = ref_value
        return _Reference(value)


def _to_json_value(value):
    if _is_reference(value):
        return {
            "reference": True,
            "value": _to_json_value(getattr(value, "value", None)),
        }
    if value is None or isinstance(value, (bool, int, float, str)):
        return value
    if isinstance(value, (bytes, bytearray)):
        return {"bytes_hex": _hex(value)}
    if isinstance(value, (list, tuple)):
        return [_to_json_value(item) for item in value]
    if isinstance(value, dict):
        return dict((str(key), _to_json_value(item)) for key, item in value.items())
    if hasattr(value, "__dict__"):
        fields = {}
        for name, item in value.__dict__.items():
            if not name.startswith("_"):
                fields[name] = _to_json_value(item)
        return {
            "type": value.__class__.__name__,
            "fields": fields,
        }
    return repr(value)


def _is_reference(value):
    return value is not None and value.__class__.__name__ == "Reference" and hasattr(value, "value")


def _get_decoded_payload(message):
    idl = message.get("idl")
    if not idl:
        return None
    return idl.get("payload")


def _close_packet_logger(logger):
    if logger is None:
        return
    close = getattr(logger, "close", None)
    if close is None:
        return
    try:
        close()
    except Exception:
        pass


def _get_bool(environ, name, default):
    value = environ.get(name)
    if value is None:
        return default

    value = value.strip().lower()
    if value in _TRUE_VALUES:
        return True
    if value in _FALSE_VALUES:
        return False
    return default


def _get_int(environ, name, default):
    value = environ.get(name)
    if value is None:
        return default

    try:
        return int(value.strip())
    except ValueError:
        return default


def _is_false(value):
    return value.strip().lower() in _FALSE_VALUES


def _split_env_list(value):
    result = []
    for item in value.replace(";", ",").split(","):
        item = item.strip()
        if item:
            result.append(item)
    return result


def _timestamp_info():
    try:
        timestamp_ns = time.time_ns()
        timestamp_sec = timestamp_ns / 1000000000.0
    except AttributeError:
        timestamp_sec = time.time()
        timestamp_ns = int(timestamp_sec * 1000000000)

    timestamp = datetime.datetime.fromtimestamp(timestamp_sec, datetime.timezone.utc).astimezone().isoformat()
    return timestamp, timestamp_ns


def _to_bytes(data):
    if data is None:
        return bytes()
    return bytes(data)


def _hex(data):
    return binascii.hexlify(_to_bytes(data)).decode("ascii")


def _hex_spaced(data):
    value = _hex(data)
    return " ".join(value[index:index + 2] for index in range(0, len(value), 2))


def _format_optional_hex(value):
    if value is None:
        return None
    return "0x%04x" % value


def _format_message(message):
    if not message:
        return ""
    if message.get("error"):
        return "error=%s" % message["error"]

    service = message.get("service")
    request = message.get("request")
    idl = message.get("idl")
    if idl:
        if idl.get("service"):
            service = "%s(%s)" % (service, idl.get("service"))
        if idl.get("method"):
            request = "%s(%s)" % (request, idl.get("method"))

    return "type=%s service=%s request=%s sequence=%s" % (
        message.get("type"),
        service,
        request,
        message.get("sequence"),
    )


atexit.register(_close_default_packet_logger)
