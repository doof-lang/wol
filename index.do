// Wake-on-LAN magic packet construction and IPv4 broadcast delivery.
//
// The receiving machine must have Wake-on-LAN enabled in its firmware and
// network adapter. Sending a packet does not prove that the machine woke.

export import isolated function wakeOnLan(macAddress: string, broadcastAddress: string = "255.255.255.255", port: int = 9): Result<none, string> from "./native_wol.hpp" as doof_wol::wake

export import isolated function magicPacket(macAddress: string): Result<byte[], string> from "./native_wol.hpp" as doof_wol::magic_packet
