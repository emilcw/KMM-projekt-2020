// This is the client code for a tcp connetion.
// Author Josef Olsson (josol381)
// and Isak Stenström (isast379)


using System;
using System.Net;
using System.Net.Sockets;
using System.Text;

namespace PCInterface
{
    public class TcpClient
    {

        private Socket s;
        private string storedMessage = "";
        public bool IsConnected { get; private set; } = false;

        /*
         * Tries to connect to a server withg IpAdress.
         */
        public bool ConnectToServer(string IpAddress)
        {
            IPEndPoint hostEp = new IPEndPoint(IPAddress.Parse(IpAddress), 1337);
            Socket tempSocket = new Socket(AddressFamily.InterNetwork, SocketType.Stream, ProtocolType.Tcp);
            try
            {
                tempSocket.Connect(hostEp);
            }
            catch (SocketException)
            {
                IsConnected = false;
                return false;
            };

            if (tempSocket.Connected)
            {
                tempSocket.Blocking = false;
                s = tempSocket;
                IsConnected = true;
                return true;
            }
            IsConnected = false;
            return false;
        }

        /*
        * Sends data to the server.
        */
        public void Send(string message)
        {
            if (IsConnected)
            {
                byte[] bytesSent = Encoding.ASCII.GetBytes(message);
                s.Send(bytesSent, bytesSent.Length, 0);
            }
            else
            {
                throw new InvalidOperationException("Cannot send data when not connected!");
            }
        }

        /*
         * Recieves data sent from the server.
         */
        public string[][] Recive()
        {
            if (IsConnected)
            {
                byte[] bytesReceived = new byte[65536];
                try
                {
                    int bytes = s.Receive(bytesReceived, bytesReceived.Length, 0);
                    string m = Encoding.ASCII.GetString(bytesReceived, 0, bytes);
                    m = storedMessage + m;
                    string[] commands = m.Split(';');
                    storedMessage = commands[commands.Length - 1];
                    Array.Resize(ref commands, commands.Length - 1);
                    string[][] commandsWithArguments = new string[commands.Length][];
                    for (int i = 0; i < commands.Length; i++)
                    {
                        commandsWithArguments[i] = commands[i].Split(',');
                    }
                    return commandsWithArguments;
                }
                catch (SocketException) { }
                return null;
            }
            else
            {
                throw new InvalidOperationException("Cannot recieve data when not connected!");
            }

        }
    }
}

