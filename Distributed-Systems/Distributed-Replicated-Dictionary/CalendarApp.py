import sys
from select import select
import json

from UdpServer import UdpServer
from event import event

"""
TODO:
    - schedule():
        - available(): check if users are available
    - update clocks

    - all message passing
        - parse_message()
        - send_message()
        - has_received()
    - actual Wuu-Berntstein algorithm
"""

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
                hosts.append((ID, int(port)))
                # if len(ID) != 25 or not ID.isalnum():
                    # print("IDs must be 25 character alphanumeric string")
                    # exit()

        # ensure our site ID is in the group of known hosts
        for host in hosts:
            if host[0] == site_id:
                port = host[1]
                break
        else:
            print("Site ID must be contained in knownhosts_udp.txt")
            exit()

        # build a matrix clock that includes all hosts,
        # an empty calendar, and assign sequential IDs
        # to each site
        clock = {}
        calendar = {}
        index = {}
        for i,host in enumerate(hosts):
            clock[host[0]] = [0] * len(hosts)
            calendar[host[0]] = []
            index[host[0]] = i

    else:
        print("Invalid Argument(s)")
        print("USAGE: {0} <site-id>".format(sys.argv[0]))
        exit()

    return site_id, int(port), hosts, clock, calendar, index

# -----------------------------------------------------------------------------
def print_matrix_clock(T):
    print("\nClock:")
    for ID, clock in T.items():
        print(ID, clock)
    print()

# -----------------------------------------------------------------------------
def parse_message(data, address):
    print("\nReceived:", data)
    print(data["01234"])

# -----------------------------------------------------------------------------
def send_message():
    pass

# -----------------------------------------------------------------------------
def available(participants, calendar, event):
    return True

# -----------------------------------------------------------------------------
def has_received(clock, participant):
    pass

# -----------------------------------------------------------------------------
def parse_command(user_input, calendar, site_id, clock, I, log_file):
    user_input = user_input.split()
    command = user_input[0]
    args = user_input[1:]

    if command.lower() == "schedule":
        schedule(args, calendar, clock, I, site_id, log_file)
    elif command.lower() == "cancel":
        cancel(args[0], calendar, clock, I, site_id, log_file)
    elif command.lower() == "view":
        view(calendar)
    elif command.lower() == "myview":
        myview(calendar, site_id)
    elif command.lower() == "log":
        view_log(log_file)
    else:
        print("ERROR: Invalid command.")

# -----------------------------------------------------------------------------
def schedule(args, calendar, clock, I, site_id, log_file):
    print("\nUser requested SCHEDULE")
    name, day, start, end = args[0:4]
    participants = args[4:]

    # be robust to whether a user inputs a list like this:
    # user1 user2 user3
    # or this:
    # user1,user2,user3
    if len(participants) == 1:
        participants = participants[0].split(",")

        # attempt to make event with user-given parameters
        e = event(name, day, start, end, participants)

    # ensure this user is included in the user list and everyone is available
    if site_id not in participants:
        print("You cannot schedule a meeting for other users.")
    elif not available(participants, calendar, e):
        print("Not all users are available at that time")
    else:

        # add the meeting to all participants schedules
        for p in participants:
            calendar[p].append(e)
            calendar[p].sort()

        log("create", e, log_file)

        # for now just increment the clock without checking to see if it was successful
        clock[site_id][I[site_id]] += 1
        print_matrix_clock(clock)
        print("Meeting {0} scheduled.".format(name))

# -----------------------------------------------------------------------------
def cancel(meeting, calendar, clock, I, site_id, log_file):
    print("\nUser requested CANCEL")
    events = len(calendar[site_id])

    # modify the event list to remove the meeting
    calendar[site_id] = [event for event in calendar[site_id] if event.name != meeting]

    if events == len(calendar[site_id]):
        print("No events were cancelled.")
    else:
        clock[site_id][I[site_id]] += 1
        print_matrix_clock(clock)
        log("delete", meeting, log_file)
        print("Meeting {0} cancelled.".format(meeting))


# -----------------------------------------------------------------------------
def view(calendar):
    print("\nUser requested VIEW")

    # get every event from the calendar, use set to avoid duplicates
    events = {e for ID, event_list in calendar.items() for e in event_list}
    events = list(events)

    # sort in lexicographical order and print all
    events.sort()
    for e in events:
        print(e)

# -----------------------------------------------------------------------------
def myview(calendar, site_ID):
    print("\nUser requested MYVIEW")
    for e in calendar[site_id]:
        print(e)
# -----------------------------------------------------------------------------
def log(action, event, log_file):
    if action == "create" or action == "delete":
        log_file.write(action + " " + str(event) + "\n")
    else:
        print("Unknown action type. Cannot log.")
# -----------------------------------------------------------------------------
def view_log(log_file):
    print("\nUser requested LOG")
    log_file.seek(0)
    print(log_file.read())

# =============================================================================
if __name__ == "__main__":
    site_id, port, hosts, clock, calendar, I = read_known_hosts()
    print_matrix_clock(clock)
    log_name = "log.txt"
    log_file = open(log_name, "r+")
    log_file.truncate(0)

    # test by adding an event to the calendar and viewing it
    args = ["Breakfast", "10/14/2018", "08:00", "09:00", site_id]
    schedule(args, calendar, clock, I, site_id, log_file)
    myview(calendar, site_id)

    # both server and poll for user input are non-blocking
    server = UdpServer("127.0.0.1", port)
    timeout = 0


    print("String:", type(json.dumps("Hello")))
    print("Dictionary:", type(json.dumps(clock)))


    # just for testing. Send a message to all known hosts
    for host, port in hosts:
        server.send("Hello from {0}".format(site_id), ("127.0.0.1", port))
        server.send(json.dumps(clock), ("127.0.0.1", port))


    print("Enter a command:")
    while True:
        # attempt to receive any message
        data, address = server.receive()
        if data:
            print("Datatype:", type(data))
            parse_message(data, address)

        # check for user input
        rlist, _, _ = select([sys.stdin], [], [], timeout)
        if rlist:
            command = sys.stdin.readline()
            if command == "quit\n" or command == "exit\n":
                print("Exiting.")
                exit()
            else:
                parse_command(command, calendar, site_id, clock, I, log_file)