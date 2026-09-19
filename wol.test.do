import { Assert } from "std/assert"
import { magicPacket } from "./index"

export function testMagicPacket(): none {
    packet := magicPacket("aa:bb:cc:dd:ee:ff")!
    Assert.equal(packet.length, 102)
    Assert.equal<byte>(packet[0], byte(255))
    Assert.equal<byte>(packet[5], byte(255))
    Assert.equal<byte>(packet[6], byte(170))
    Assert.equal<byte>(packet[11], byte(255))
    Assert.equal<byte>(packet[96], byte(170))
    Assert.equal<byte>(packet[101], byte(255))
}

export function testMagicPacketRejectsInvalidMac(): none {
    result := magicPacket("not-a-mac")
    failed := case result {
        _: Success -> false,
        _: Failure -> true,
    }
    Assert.isTrue(failed)
}
