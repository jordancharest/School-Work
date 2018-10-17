import socket
import pickle



class UdpServer:
    def __init__(self, host, port):
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        self.host = host
        self.port = port
        self.sock.bind((self.host, self.port))
        # self.sock.settimeout(0.25)
        self.sock.setblocking(0)

    def send(self, data, address):
        self.sock.sendto(pickle.dumps(data), address)

    def receive(self, size=1024):
        """
        Non-blocking receive because I don't want to do multi-threading in
        Python. Not sure how well it will work. Throws socket.error if there
        are no messages in the network buffer
        """
        try:
            data, address = self.sock.recvfrom(size)
        except socket.error:
            return None, None
        else:
            return pickle.loads(data), address


# Ignore
if __name__ == "__main__":
    # run tests on the UDP server object
    host = "192.168.0.1"
    port = 5000
     
    mySocket = socket.socket()
    mySocket.bind((host,port))
     
    mySocket.listen(1)
    conn, addr = mySocket.accept()
    print ("Connection from: " + str(addr))
    while True:
            data = conn.recv(1024).decode()
            if not data:
                    break
            print ("from connected  user: " + str(data))
             
            data = str(data).upper()
            print ("sending: " + str(data))
            conn.send(data.encode())
             
    conn.close()
