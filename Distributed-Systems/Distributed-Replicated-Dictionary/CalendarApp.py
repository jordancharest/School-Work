import sys
from select import select
import json
import socket

from UdpServer import UdpServer
from event import Event

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
                IP, port = line.split()
                hosts.append((IP, int(port)))
                # if len(ID) != 25 or not ID.isalnum():
                    # print("IDs must be 25 character alphanumeric string")
                    # exit()

        # ensure our site ID is in the group of known hosts
        else:
            print("Site ID must be contained in knownhosts_udp.txt")
            exit()
            
        # find the index of the host
        site_index = 0
        for host in hosts:
            site_index = site_index + 1
            if host[0] == site_id:
                ip = host[0]
                port = host[1]
                break

        # build a matrix clock that includes all hosts,
        # an empty calendar, and assign sequential IDs
        # to each site
        clock = {}
        calendar = []
        PL = []
        for i,host in enumerate(hosts):
            clock[host[0]] = [0] * len(hosts)

    else:
        print("Invalid Argument(s)")
        print("USAGE: {0} <site-id>".format(sys.argv[0]))
        exit()

    return site_id, site_index, ip, int(port), hosts, clock, calendar, PL

# -----------------------------------------------------------------------------
def print_matrix_clock(site_id, T):
    print("\n{0} clock:".format(site_id))
    for ID, clock in T.items():
        print(ID, clock)
    print()

# -----------------------------------------------------------------------------
def parse_message(data, address):
    print("\nReceived:", data)
    print(data[1])

# -----------------------------------------------------------------------------
def send_message(PL, clock, receiver_id, server, ip, port):
    NP = []
    for eR in PL:
        if not hasRec(clock, eR, receiver_id):
            NP.append(eR)
            
    msg = {
        'NP':NP,
        'clock': clock
    }
    
    server.send(json.dumps(msg), (ip, port))
    
# -----------------------------------------------------------------------------
def receive_message(data, address, calendar, receiver_id, receiver_index, clock, PL, hosts):
    # convert data
    data_dict = json.loads(data)
    Tk = data_dict['clock']
    NP = data_dict['NP']
    
    # compute NE
    NE = []
    for fR in NP:
        if not hasRec(clock, fR, receiver_id):
            NE.append(fR)
    
    # update dictionary 
    for dR in NE:
        tmp0, tmp1, usr_cmd, meeting = dR.split(" ", 3)
        PL.append(dR)
        if usr_cmd == "create":                      
            e = Event.load(meeting)  
            calendar.append(e)
            calendar.sort()
    for dR in NE:
        tmp0, tmp1, usr_cmd, meeting = dR.split(" ", 3)        
        if usr_cmd == "delete":
            name = meeting
            calendar = [event for event in calendar if event.name != name]     
            
    
    # host info
    n_hosts = 0
    sender_id = None
    sender_index = 0
    for host in hosts:
            n_hosts = n_hosts + 1
            #print(host[0])
            ip = socket.gethostbyname(host[0])
            #print(ip)
            port = host[1]
            #print(address[0])
            if  ip == address[0] and port == address[1]:
                sender_id = host[0]
                sender_index = n_hosts
    
    # update matrix clock step 1
    for r in range(1,n_hosts):
        clock[str(receiver_id)][r-1] = max(clock[str(receiver_id)][r-1],Tk[str(sender_id)][r-1])
    
    # update matrix clock step 2
    for r in range(1,n_hosts):
        for s in range(1,n_hosts):
            clock[hosts[r-1][0]][s-1] = max(clock[hosts[r-1][0]][s-1],int(Tk[hosts[r-1][0]][s-1]))
    
    # update the partial log 
    PL_temp = PL.copy()
    PL = []
    for eR in PL_temp:
        flag = False
        for s in range (1,n_hosts):
            if not hasRec(clock, eR, hosts[s-1][0]):
                flag = True 
                break
        if flag:
            PL.append(eR) 
    return calendar
    # End of the function

# -----------------------------------------------------------------------------
def available(participants, calendar, event):
    flag_avail = True
    for p in participants:
        for e in calendar:
            if flag_avail:
                for line in e.participants:
                    ep = line.split(",")
                    if p in ep:
                        if event.date != e.date:
                            break
                        elif event.end <= e.start:
                            break
                        elif event.start >= e.end:
                            break
                        else:
                            flag_avail = False
                            break
    
    return flag_avail

# -----------------------------------------------------------------------------
def has_received(clock, participant):
    pass

# -----------------------------------------------------------------------------
def parse_command(user_input, calendar, site_id, site_index, clock, PL, server, hosts):
    user_input = user_input.split()
    command = user_input[0]
    command = command.lower()
    args = user_input[1:]

    if command == "create" or command == "schedule":
        schedule(args, calendar, clock, site_id, site_index, PL)
        participants = args[4:]
        if len(participants) == 1:
            participants = participants[0].split(",")
        ct = 0
        for ip, port in hosts: 
            ct = ct + 1         
            if ct != site_index:
                if ip in participants:                    
                    send_message(PL, clock, ip, server, ip, port)
    elif command == "cancel":
        participants = []
        for item in calendar:          
            if item.name == args[0]:
                participants = item.participants
                break                
        if len(participants) == 1:
            participants = participants[0].split(",")
                        
        calendar = cancel(args[0], calendar, clock, site_id, site_index, PL)
        
        ct = 0
        for ip, port in hosts: 
            ct = ct + 1         
            if ct != site_index:
                if ip in participants:                    
                    send_message(PL, clock, ip, server, ip, port)
    elif command == "view":
        view(calendar)
    elif command == "myview":
        myview(calendar, site_id)
    elif command == "log":
        view_log(PL)
    elif command == "clock":
        print_matrix_clock(site_id, clock)
    else:
        print("ERROR: Invalid command.")
        
    return(calendar)

# -----------------------------------------------------------------------------
def schedule(args, calendar, clock, site_id, site_index, PL):
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
        e = Event(name, day, start, end, participants)

    # ensure this user is included in the user list and everyone is available
    if site_id not in participants:
        print("You cannot schedule a meeting for other users.")
    elif not available(participants, calendar, e):
        print("Not all users are available at that time")
    else: # insert(x)
      
        # for now just increment the clock without checking to see if it was successful
        clock[str(site_id)][site_index-1] += 1
        #print_matrix_clock(clock)
        
        # update log
        log("create", e, clock[str(site_id)][site_index-1], site_index, PL)
        
        # add x to this site's schedule   
        calendar.append(e)
        calendar.sort()
        
        

# -----------------------------------------------------------------------------
def cancel(meeting, calendar, clock, site_id, site_index, PL):
    print("\nUser requested CANCEL")
    events = len(calendar)

    # modify the event list to remove the meeting
    calendar = [event for event in calendar if event.name != meeting]
    #print(calendar)

    if events == len(calendar):
        print("No events were cancelled.")
    else:
        clock[str(site_id)][site_index-1] += 1
        print_matrix_clock(site_id, clock)
        log("delete", meeting, clock[str(site_id)][site_index-1], site_index, PL)
        print("Meeting {0} cancelled.".format(meeting))
    return calendar


# -----------------------------------------------------------------------------
def view(calendar):
    print("\nUser requested VIEW")

    # get every event from the calendar, use set to avoid duplicates
    events = {e for e in calendar}
    events = list(events)

    # sort in lexicographical order and print all
    events.sort()
    for e in events:
        print(e)

# -----------------------------------------------------------------------------
def myview(calendar, site_ID):
    print("\nUser requested MYVIEW")
    for e in calendar:
        for p in e.participants:
            p = p.split(",")
            if site_ID in p:
                print(e)
                break
            
        
# -----------------------------------------------------------------------------
def log(action, event, Ci, i, PL):
    if action == "create" or action == "delete":
        PL.append(str(i) + " " + str(Ci) + " " + action + " " + str(event))
    else:
        print("Unknown action type. Cannot log.")
# -----------------------------------------------------------------------------
def view_log(PL):
    print("\nUser requested LOG")
    for item in PL:
        i,Ci,logEntry = item.split(" ",2) 
        print(logEntry)
        
# -----------------------------------------------------------------------------
def hasRec(Ti, eR, k):
    i,Ci,logEntry = eR.split(" ",2) 
    #print(k)
    return Ti[str(k)][int(i)-1] >= int(Ci)
    

# =============================================================================
if __name__ == "__main__":
    site_id, site_index, ip, port, hosts, clock, calendar, PL = read_known_hosts()
    print_matrix_clock(site_id, clock)
    

    # both server and poll for user input are non-blocking
    server = UdpServer(ip, port)
    timeout = 0.25
    
    
    # test by adding an event to the calendar and viewing it
#    args = ["Breakfast", "10/14/2018", "08:00", "09:00", site_id]
#    schedule(args, calendar, clock, site_id, PL, server, hosts)
#    myview(calendar, site_id)

    


#    print("String:", type(json.dumps("Hello")))
#    print("Dictionary:", type(json.dumps(clock)))


    # just for testing. Send a message to all known hosts
#    for ip, port in hosts:
#        server.send("Hello from {0}".format(site_id), (ip, port))
#        server.send(json.dumps(clock), (ip, port))


    print("Enter a command:")
    while True:
        # attempt to receive any message
        data, address = server.receive()
        if data:
            calendar = receive_message(data, address, calendar, site_id, site_index, clock, PL, hosts)

        # check for user input
        rlist, _, _ = select([sys.stdin], [], [], timeout)
        if rlist:
            command = sys.stdin.readline()
            if command == "quit\n" or command == "exit\n":
                print("Exiting.")
                exit()
            else:
                calendar = parse_command(command, calendar, site_id, site_index, clock, PL, server, hosts)
