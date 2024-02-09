package  io.github.embeddedrpc.erpc.codec;

import  io.github.embeddedrpc.erpc.auxiliary.MessageInfo;
import  io.github.embeddedrpc.erpc.auxiliary.MessageType;
import  io.github.embeddedrpc.erpc.auxiliary.Utils;
import java.util.Arrays;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertArrayEquals;
import static org.junit.jupiter.api.Assertions.assertDoesNotThrow;
import static org.junit.jupiter.api.Assertions.assertEquals;
import static org.junit.jupiter.api.Assertions.assertThrows;

/**
 * Unit tests for BasicCodec.
 */
@SuppressWarnings("magicnumber")
class BasicCodecTest {
    /**
     * Rigorous Test.
     */

    @Test
    void boolTest() {
        Codec codec = new BasicCodec();

        // Push data to codec
        codec.writeBool(false);
        codec.writeBool(true);

        codec.getBuffer().position(0);

        assertEquals(false, codec.readBool());
        assertEquals(true, codec.readBool());
    }

    @Test
    void int8Test() {
        Codec codec = new BasicCodec();

        // Push data to codec
        codec.writeInt8((byte) 42);
        codec.writeInt8((byte) -42);
        codec.writeUInt8((byte) 42);

        assertThrows(IllegalArgumentException.class, () -> {
            codec.writeUInt8((byte) -42);
        });
        assertThrows(IllegalArgumentException.class, () -> {
            codec.writeUInt8((byte) 0xff);
        });
        assertDoesNotThrow(() -> {
            codec.writeUInt8((byte) 0);
        });
        assertDoesNotThrow(() -> {
            codec.readUInt8();
        });

        // Reset buffer position
        codec.getBuffer().position(0);

        // Read data from codec
        assertEquals(42, codec.readInt8());
        assertEquals(-42, codec.readInt8());
        assertEquals(42, codec.readUInt8());
    }

    @Test
    void int16Test() {
        Codec codec = new BasicCodec();

        // Push data to codec
        codec.writeInt16((short) 42);
        codec.writeInt16((short) -42);
        codec.writeUInt16((short) 42);

        assertThrows(IllegalArgumentException.class, () -> {
            codec.writeUInt16((short) -42);
        });
        assertThrows(IllegalArgumentException.class, () -> {
            codec.writeUInt16((short) 0xffff);
        });
        assertDoesNotThrow(() -> {
            codec.writeUInt16((short) 0);
        });
        assertDoesNotThrow(() -> {
            codec.readUInt16();
        });

        // Reset buffer position
        codec.getBuffer().position(0);

        // Read data from codec
        assertEquals(42, codec.readInt16());
        assertEquals(-42, codec.readInt16());
        assertEquals(42, codec.readUInt16());
    }

    @Test
    void int32Test() {
        Codec codec = new BasicCodec();

        // Push data to codec
        codec.writeInt32(42);
        codec.writeInt32(-42);
        codec.writeUInt32(42);

        assertThrows(IllegalArgumentException.class, () -> {
            codec.writeUInt32(-42);
        });
        assertThrows(IllegalArgumentException.class, () -> {
            codec.writeUInt32(0xffffffff1L);
        });
        assertDoesNotThrow(() -> {
            codec.writeUInt32(0);
        });
        assertDoesNotThrow(() -> {
            codec.readUInt32();
        });

        // Reset buffer position
        codec.getBuffer().position(0);

        // Read data from codec
        assertEquals(42, codec.readInt32());
        assertEquals(-42, codec.readInt32());
        assertEquals(42, codec.readUInt32());
    }

    @Test
    void int64Test() {
        Codec codec = new BasicCodec();

        // Push data to codec
        codec.writeInt64(42);
        codec.writeInt64(-42);

        // Reset buffer position
        codec.getBuffer().position(0);

        // Read data from codec
        assertEquals(42, codec.readInt64());
        assertEquals(-42, codec.readInt64());
    }

    @Test
    void floatTest() {
        Codec codec = new BasicCodec();

        // Push data to codec
        codec.writeFloat(42);
        codec.writeFloat(-42);
        codec.writeFloat(0.125F);
        codec.writeFloat(-0.125F);

        // Reset buffer position
        codec.getBuffer().position(0);

        // Read data from codec
        assertEquals(42, codec.readFloat());
        assertEquals(-42, codec.readFloat());
        assertEquals(0.125F, codec.readFloat());
        assertEquals(-0.125F, codec.readFloat());
    }

    @Test
    void doubleTest() {
        Codec codec = new BasicCodec();

        // Push data to codec
        codec.writeDouble(42);
        codec.writeDouble(-42);
        codec.writeDouble(0.125F);
        codec.writeDouble(-0.00000125F);

        // Reset buffer position
        codec.getBuffer().position(0);

        // Read data from codec
        assertEquals(42, codec.readDouble());
        assertEquals(-42, codec.readDouble());
        assertEquals(0.125F, codec.readDouble());
        assertEquals(-0.00000125F, codec.readDouble());
    }

    @Test
    void stringTest() {
        Codec codec = new BasicCodec();

        // Push data to codec
        codec.writeString("HelloWord");
        codec.writeString("+ěščřžýáíé=");
        codec.writeString("Ḽơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť ӓṁệẗ, ĉṓɲṩḙċťᶒțûɾ ấɖḯƥĭṩčįɳġ ḝłįʈ");

        // Reset buffer position
        codec.getBuffer().position(0);

        // Read data from codec
        assertEquals("HelloWord", codec.readString());
        assertEquals("+ěščřžýáíé=", codec.readString());
        assertEquals("Ḽơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť ӓṁệẗ, ĉṓɲṩḙċťᶒțûɾ ấɖḯƥĭṩčįɳġ ḝłįʈ", codec.readString());

    }

    @Test
    void longStringTest() {
        // Testing data
        String testString = "Ḽơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť ӓṁệẗ, ĉṓɲṩḙċťᶒțûɾ ấɖḯƥĭṩčįɳġ ḝłįʈḼơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť ӓṁệẗ, ĉṓɲṩḙċť"
                + "ᶒțûɾ ấɖḯƥĭṩčįɳġ ḝłįḼơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť ӓṁệẗ, ĉṓɲṩḙċťᶒțûɾ ấɖḯƥĭṩčįɳġ ḝłįḼơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť ӓṁệẗ,"
                + "ĉṓɲṩḙċťᶒțûɾ ấɖḯƥĭṩčįɳġ ḝłįḼơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť ӓṁệẗ, ĉṓɲṩḙċťᶒțûɾ ấɖḯƥĭṩčįɳġ ḝłįḼơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐ"
                + "ť ӓṁệẗ, ĉṓɲṩḙċťᶒțûɾ ấɖḯƥĭṩčįɳġ ḝłįḼơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť ӓṁệẗ, ĉṓɲṩḙċťᶒțûɾ ấɖḯƥĭṩčįɳġ ḝłįḼơᶉëᶆ ȋṕšᶙṁ "
                + "ḍỡḽǭᵳ ʂǐť ӓṁệẗ, ĉṓɲṩḙċťᶒțûɾ ấɖḯƥĭṩčįɳġ ḝłį";

        // New codec
        Codec codec = new BasicCodec();

        // Push data to codec
        codec.writeString(testString);

        // Reset buffer position
        codec.getBuffer().position(0);

        // Read data from codec

        assertEquals(testString, codec.readString());

    }

    @Test
    void longBinaryTest() {
        // Testing data
        byte[] array = new byte[65536];
        Arrays.fill(array, (byte) 42);

        Codec codec = new BasicCodec();

        // Push data to codec
        codec.writeBinary(array);
        codec.writeBinary(array);
        codec.writeBinary(array);

        // Reset buffer position
        codec.getBuffer().position(0);

        // Read data from codec

        assertArrayEquals(array, codec.readBinary());
        assertArrayEquals(array, codec.readBinary());
        assertArrayEquals(array, codec.readBinary());
    }

    @Test
    void byteRepresentationTest() {
        Codec codec = new BasicCodec();

        // Push data to codec
        codec.writeBool(true);
        codec.writeBool(false);

        codec.writeInt8((byte) -42);
        codec.writeInt8((byte) 42);
        codec.writeUInt8((short) 42);

        codec.writeInt16((byte) -42);
        codec.writeInt16((byte) 42);
        codec.writeUInt16((short) 42);

        codec.writeInt32((byte) -42);
        codec.writeInt32((byte) 42);
        codec.writeUInt32((short) 42);

        codec.writeString("Ḽơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť ӓṁệẗ");

        // Get bytes length
        int dataLength = codec.getBuffer().position();

        // Reset buffer position
        codec.getBuffer().position(0);

        // Read data from codec
        byte[] array = new byte[dataLength];
        codec.getBuffer().get(array, 0, dataLength);

        // Assert equality (bytes string from Python implementation)
        assertEquals("0100d62a2ad6ff2a002a00d6ffffff2a0000002a0000003d000000e1b8bcc6a1e1b689c3abe1b68620c88be1b995c5a1e"
                        + "1b699e1b98120e1b88de1bba1e1b8bdc7ade1b5b320ca82c790c5a520d393e1b981e1bb87e1ba97",
                Utils.byteArrayToHex(array));
    }

    @Test
    void fromBytesTest() {
        String testString = "0100d62a2ad6ff2a002a00d6ffffff2a0000002a0000003d000000e1b8bcc6a1e1b689c3abe1b68620c88be1b9"
                + "95c5a1e1b699e1b98120e1b88de1bba1e1b8bdc7ade1b5b320ca82c790c5a520d393e1b981e1bb87e1ba97";
        Codec codec = new BasicCodec(Utils.hexToByteArray(testString));

        assertEquals(true, codec.readBool());
        assertEquals(false, codec.readBool());
        assertEquals(-42, codec.readInt8());
        assertEquals(42, codec.readInt8());
        assertEquals(42, codec.readUInt8());
        assertEquals(-42, codec.readInt16());
        assertEquals(42, codec.readInt16());
        assertEquals(42, codec.readUInt16());
        assertEquals(-42, codec.readInt32());
        assertEquals(42, codec.readInt32());
        assertEquals(42, codec.readUInt32());
        assertEquals("Ḽơᶉëᶆ ȋṕšᶙṁ ḍỡḽǭᵳ ʂǐť ӓṁệẗ", codec.readString());
    }

    @Test
    void writeMessageTest() {
        Codec codec = new BasicCodec();
        MessageInfo msgInfo = new MessageInfo(MessageType.kNotificationMessage, 1, 5, 4);

        codec.startWriteMessage(msgInfo);

        // Reset buffer position
        codec.getBuffer().position(0);

        MessageInfo msgInfoRead = codec.startReadMessage();

        // Assert equality
        assertEquals(msgInfo, msgInfoRead);
    }

    @Test
    void readMessageTest() {
        String testString = "020101012a000000";
        MessageInfo msgInfoExpected = new MessageInfo(MessageType.kReplyMessage, 1, 1, 42);

        Codec codec = new BasicCodec(Utils.hexToByteArray(testString));

        MessageInfo msgInfo = codec.startReadMessage();

        assertEquals(msgInfoExpected, msgInfo);
    }

    @Test
    void codecToArrayTest() {
        Codec codec = new BasicCodec();

        codec.writeInt32(1);

        byte[] array = codec.array();

        assertEquals(4, array.length);
        assertEquals(1, array[0]);
        assertEquals(0, array[1]);
        assertEquals(0, array[2]);
        assertEquals(0, array[3]);

        codec.writeUInt32(0xFFFFFFFFL);

        array = codec.array();

        assertEquals(8, array.length);
        assertEquals(1, array[0]);
        assertEquals(0, array[1]);
        assertEquals(0, array[2]);
        assertEquals(0, array[3]);
        assertEquals((byte) 0xFF, array[4]);
        assertEquals((byte) 0xFF, array[5]);
        assertEquals((byte) 0xFF, array[6]);
        assertEquals((byte) 0xFF, array[7]);

    }
}
