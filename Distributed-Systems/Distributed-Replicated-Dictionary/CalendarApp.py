import sys
from select import select

import UdpServer as udp

# -----------------------------------------------------------------------------
def read_known_hosts():
    """
    Read all hosts from text file and ensure that the given site ID is in
    the group of known hosts. Otherwise other sites will not know of this
    site's existence.
    """
    if len(sys.argv) == 2:
        _, site_id = sys.argv

        # read all known hosts
        file = open("knownhosts_udp.txt", "r")
        hosts = []
        if file.mode == "r":
            lines = file.readlines()
            for line in lines:
                host, port = line.split()
                hosts.append((host, port))

        # ensure our site ID is in the group of known hosts
        for host in hosts:
            if host[0] == site_id:
                port = host[1]
                break
        else:
            print("Site ID must be contained in knownhosts_udp.txt")
            exit()


    else:
        print("Invalid Argument(s)")
        print("USAGE: {0} <site-id>".format(argv[0]))
        exit()

    return site_id, int(port), hosts

# -----------------------------------------------------------------------------
def parse_messaage(data, address):
    print("Received message")

# -----------------------------------------------------------------------------
def parse_command(user_input):
    user_input = user_input.split()
    command = user_input[0]
    args = user_input[1:]

    if command.lower() == "schedule":
        schedule(args)
    elif command.lower() == "cancel":
        cancel(args)
    elif command.lower() == "view":
        view()
    elif command.lower() == "myview":
        myview()
    elif command.lower() == "log":
        log()
    else:
        print("ERROR: Invalid command.")

# -----------------------------------------------------------------------------
def schedule(args):
    print("User requested SCHEDULE")

# -----------------------------------------------------------------------------
def cancel(meeting):
    print("User requested CANCEL")

# -----------------------------------------------------------------------------
def view():
    print("User requested VIEW")

# -----------------------------------------------------------------------------
def myview():
    print("User requested MYVIEW")

# -----------------------------------------------------------------------------
def log():
    print("User requested LOG")

# =============================================================================
if __name__ == "__main__":
    site_id, port, hosts = read_known_hosts()

    # both server and poll for user input are non-blocking
    server = udp.UdpServer(site_id, port)
    timeout = 0
    print("Enter a command:")

    while True:
        # attempt to receive any message
        data, address = server.receive()
        if data:
            parse_message(data, address)

        # check for user input
        rlist, _, _ = select([sys.stdin], [], [], timeout)
        if rlist:
            command = sys.stdin.readline()
            if command == "quit\n" or command == "exit\n":
                print("Exiting.")
                exit()
            else:
                parse_command(command)