from scapy.all import ARP, Ether, srp

def get_mac_addresses(ip_range):
    # Create an ARP request packet
    arp = ARP(pdst=ip_range)
    ether = Ether(dst="ff:ff:ff:ff:ff:ff")
    packet = ether/arp

    # Send the packet and capture the response
    result = srp(packet, timeout=3, verbose=0)[0]

    # Extract MAC addresses from the response
    devices = []
    for sent, received in result:
        devices.append({'ip': received.psrc, 'mac': received.hwsrc})

    return devices

if __name__ == "__main__":
    # Replace '192.168.1.1/24' with your network's IP range
    ip_range = '192.168.1.1/24'
    devices = get_mac_addresses(ip_range)
    
    print("Connected devices:")
    for device in devices:
        print(f"IP: {device['ip']}, MAC: {device['mac']}")
