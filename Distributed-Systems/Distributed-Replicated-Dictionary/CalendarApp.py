import sys
from select import select


from UdpServer import UdpServer
from event import event

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
                ID, port = line.split()
                hosts.append((ID, port))
                if len(ID) != 25 or not ID.isalnum():
                    print("IDs must be 25 character alphanumeric string")
                    exit()

        # ensure our site ID is in the group of known hosts
        for host in hosts:
            if host[0] == site_id:
                port = host[1]
                break
        else:
            print("Site ID must be contained in knownhosts_udp.txt")
            exit()

        # build a matrix clock that includes all hosts; each clock gets a
        # vector of zeros with a length of len(hosts)
        # also build a calendar and leave it empty
        clock = {}
        calendar = {}
        for host in hosts:
            clock[host[0]] = [0] * len(hosts)
            calendar[host[0]] = []

    else:
        print("Invalid Argument(s)")
        print("USAGE: {0} <site-id>".format(sys.argv[0]))
        exit()

    return site_id, int(port), hosts, clock, calendar

# -----------------------------------------------------------------------------
def print_matrix_clock(T):
    print("\nClock:")
    for ID, clock in T.items():
        print(ID, clock)
    print()

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
    site_id, port, hosts, clock = read_known_hosts()
    print_matrix_clock(clock)
    view(calendar)

    # both server and poll for user input are non-blocking
    server = UdpServer("127.0.0.1", port)
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