from sys import argv

import UdpServer as udp

# -----------------------------------------------------------------------------
def read_known_hosts():
    """
    Read all hosts from text file and ensure that the given site ID is in
    the group of known hosts. Otherwise other sites will not know of this
    site's existence.
    """
    if len(argv) == 2:
        _, site_id = argv

        # read all known hosts
        file = open("knownhosts_udp.txt", "r")
        hosts = []
        if file.mode == "r":
            lines = file.readlines()
            for line in lines:
                host, port = line.split()
                hosts.append((host, port))

        # ensure our site ID is in the group of known hosts
        if not any(host[0] == site_id for host in hosts):
            print("Site ID must be contained in knownhosts_udp.txt")
            exit()

        return site_id, hosts

    else:
        print("Invalid Argument(s)")
        print("USAGE: {0} <site-id>".format(argv[0]))
        exit()



# =============================================================================
if __name__ == "__main__":
    site_id, hosts = read_known_hosts()
