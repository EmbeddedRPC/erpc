package  io.github.embeddedrpc.erpc.codec;

import  io.github.embeddedrpc.erpc.auxiliary.Crc16;
import  io.github.embeddedrpc.erpc.auxiliary.Utils;
import org.junit.jupiter.api.Test;

import static org.junit.jupiter.api.Assertions.assertEquals;

/**
 * Unit tests for CRC16.
 */
@SuppressWarnings("magicnumber")
public class CRC16Test {
    private final Crc16 crc = new Crc16();

    @Test
    void crc16ByteBufferTest() {
        assertEquals(0x4547, crc.computeCRC16(Utils.hexToByteArray("5b108fe061377bb0844f6a469b7b2544")));
        assertEquals(0xf033, crc.computeCRC16(Utils.hexToByteArray("6ebe6f4b01a33686310102398daf883f")));
        assertEquals(0x60bc, crc.computeCRC16(Utils.hexToByteArray("82121c9510a01971366a71fc46c27eff")));
        assertEquals(0xd127, crc.computeCRC16(Utils.hexToByteArray("311275c4ce315456aea1a75993403be3")));
        assertEquals(0x2fba, crc.computeCRC16(Utils.hexToByteArray("4fd3abc6b911c737c66f750f55fc4216")));
        assertEquals(0x45a, crc.computeCRC16(Utils.hexToByteArray("5a63996ab77d2202f480687069ca8ffe")));
        assertEquals(0x4a0a, crc.computeCRC16(Utils.hexToByteArray("b8dbfee5a566214c0f1b39a4028d9b20")));
        assertEquals(0x9a37, crc.computeCRC16(Utils.hexToByteArray("6ba116397d22b71869581742e3886867")));
        assertEquals(0x8af4, crc.computeCRC16(Utils.hexToByteArray("00de0fad00cc885707a2b13ce999eb1c")));
        assertEquals(0x7a0, crc.computeCRC16(Utils.hexToByteArray("da61c52377ca5bb717de30f44df43cb3")));
        assertEquals(0x82fa, crc.computeCRC16(Utils.hexToByteArray("00396c95c8733faa47ee70aeaa123942")));
        assertEquals(0xde52, crc.computeCRC16(Utils.hexToByteArray("99ce4dbe8a0588c03f81a071b6df26e1")));
        assertEquals(0x3429, crc.computeCRC16(Utils.hexToByteArray("72f68bc19da85b9e077c46d8a190d497")));
        assertEquals(0x6e45, crc.computeCRC16(Utils.hexToByteArray("aa6ca4918b16fac5c69c463da851edb3")));
        assertEquals(0x5df, crc.computeCRC16(Utils.hexToByteArray("373610cb7d89a2c52089bb7cad7603ae")));
        assertEquals(0xb3d7, crc.computeCRC16(Utils.hexToByteArray("d47cdd4425e5e96b70f8ff0c15716433")));
        assertEquals(0x451, crc.computeCRC16(Utils.hexToByteArray("b8337e68949d675e71e27340a18d1d2b")));
        assertEquals(0x8e36, crc.computeCRC16(Utils.hexToByteArray("1fe2ae3bdb44afbe591d777ce9a0a352")));
        assertEquals(0xcd8c, crc.computeCRC16(Utils.hexToByteArray("eab6d63286db5d7b5d33fa3193ec1650")));
        assertEquals(0xa92c, crc.computeCRC16(Utils.hexToByteArray("14d39b146713049a646cb16e812fa04a")));
        assertEquals(0x492d, crc.computeCRC16(Utils.hexToByteArray("7b7e00e55ed3ec0dc12ad60ff9d5d2cf")));
        assertEquals(0x88ba, crc.computeCRC16(Utils.hexToByteArray("9f10125c276cdc518b4d61fe2ec7d5fa")));
        assertEquals(0x247a, crc.computeCRC16(Utils.hexToByteArray("a0a5763a92b232b886f95094f50c95b4")));
        assertEquals(0x751, crc.computeCRC16(Utils.hexToByteArray("1db2fa23acbf6c6bc60e6a4d8f1b6266")));
        assertEquals(0xa7ba, crc.computeCRC16(Utils.hexToByteArray("b7286f6879db13d871bc9b06aeee8932")));
        assertEquals(0x2082, crc.computeCRC16(Utils.hexToByteArray("e067284662792f25583655e547a07227")));
        assertEquals(0xa92c, crc.computeCRC16(Utils.hexToByteArray("2615f97b172ff6b8799f88afddd1e189")));
        assertEquals(0xfbb9, crc.computeCRC16(Utils.hexToByteArray("df70b5e237c110f452b1acc965140911")));
        assertEquals(0xaf5, crc.computeCRC16(Utils.hexToByteArray("d5f91e44cb9be394e5831d3d291eee7c")));
        assertEquals(0x9209, crc.computeCRC16(Utils.hexToByteArray("5e74de47e74fc901fb76e278f9abb541")));
        assertEquals(0x7a2e, crc.computeCRC16(Utils.hexToByteArray("416c54f49c8dcf093d72cc8a3aa195c9")));
        assertEquals(0xe915, crc.computeCRC16(Utils.hexToByteArray("d0593cb671d8899448f603863aca5c0b")));
        assertEquals(0xff91, crc.computeCRC16(Utils.hexToByteArray("a106b5858d9e5464eb01a388e4829f36")));
        assertEquals(0x6524, crc.computeCRC16(Utils.hexToByteArray("21705e23f29cb1465db3f410a887bf4f")));
        assertEquals(0xa82e, crc.computeCRC16(Utils.hexToByteArray("8d39ccf4c244963a29c6dd531f8861f9")));
        assertEquals(0x7765, crc.computeCRC16(Utils.hexToByteArray("8a31810b0c634ff15e5540a36b075504")));
        assertEquals(0x491, crc.computeCRC16(Utils.hexToByteArray("b48ac1deffbbc515f82508408470344a")));
        assertEquals(0x8435, crc.computeCRC16(Utils.hexToByteArray("265d6a6e206aa888190a512a9120f2d3")));
        assertEquals(0x2a56, crc.computeCRC16(Utils.hexToByteArray("514a168af0a8cd99145e0cab1f311707")));
        assertEquals(0x39f1, crc.computeCRC16(Utils.hexToByteArray("89cf0b02699b14c375b6c21fef58b572")));
        assertEquals(0x7e5d, crc.computeCRC16(Utils.hexToByteArray("7ea6196fe85f569065957e14206d8f75")));
        assertEquals(0x912a, crc.computeCRC16(Utils.hexToByteArray("fdd1e68dcfae80ae3dad3aefbe7ef158")));
        assertEquals(0x395e, crc.computeCRC16(Utils.hexToByteArray("4cf3ee7c96b9d3679295b2cb93a979bf")));
        assertEquals(0xea51, crc.computeCRC16(Utils.hexToByteArray("b6f3691f7401ed685f23ace4f7b3b3db")));
        assertEquals(0x1b39, crc.computeCRC16(Utils.hexToByteArray("0e86d611995e8a2ed6c4b0e0d97304a5")));
        assertEquals(0x530c, crc.computeCRC16(Utils.hexToByteArray("70dc6ed45f9410813dfd1600629a6080")));
        assertEquals(0xc2df, crc.computeCRC16(Utils.hexToByteArray("dc96ad1d88643f01df321e9a6fa43e0c")));
        assertEquals(0xd9e7, crc.computeCRC16(Utils.hexToByteArray("ab5cc581ff755a28aa91bc1a23272630")));
        assertEquals(0xf1de, crc.computeCRC16(Utils.hexToByteArray("24db03e36048be8da3b268fd7d7580f5")));
        assertEquals(0xf1ad, crc.computeCRC16(Utils.hexToByteArray("d66c1a23d5bf97808c662a595a474125")));
    }

    @Test
    void crc16StringTest() {
        assertEquals(0x89ac, crc.computeCRC16("123456789".getBytes()));
    }
}
