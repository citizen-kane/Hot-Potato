Hot Potato Routing Game
-----------------------

Implement centralised inter-process hot potato game. A master setups the network between n players. Once the network is setup,
master sends the POTATO packet. On each hop, player appends it's ID and decrements the HOP count. If ZERO hops are left, that
player becomes IT. This IT player then returns the packet to the master. Master needs to print the path trace and then shuts
down the network.

1. Socketlib:
- Setups the servers and client sockets
- Start the listener thread 
- Report connection related events through registered callbacks
- Sends message received to Potato_Protocol

2. Potato_Protcol:
- Create protocol message
- Parses messages received 
- Sends parsed info to Master through registered callbacks
- Types of Control Messages: 
    - LEFTPORT
        - Sent from Player to Master to report about the port it's listening on
    - RIGHTCONNECTED
        - ACK send by Player to Master when it's connected to both peers
    - PLAYERID
        - Master sends the player ID and neighbor IDs to each player on connection
    - RIGHTINFO
        - Once all players connect, Master sends each player info about their right peers sockets. Player connects to this and reports with RIGHTCONNECTED message
    - SHUTDOWN
        - Once the potato returns to Master, it'll gracefully shutdown each player and then itself.


