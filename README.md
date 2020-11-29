# vote_server
Crossplatform server for secret vote

Secure e-Voting Server is a main part of secret ballot election and ensures anonimous vote.
 
Strong cryptographic protocol He-Su [1] is used that provides reliable protection against most threats.
ATE pairing on the elliptic curve BN254 is used to implement Chaum's blind signatures (based on BLS signatures).
For personal authentication of clients a SPEKE protocol is used that provides zero knowledge  to prevent off-line password attacks.
This protocol is implemented on an elliptic curve x25519; the elligator2 algorithm is used for hashing to the curve.
To identify the user an invitation in the form of a QR-code is used containing the user's number in the personal list, his password and the onion-address of the server.

He-Su protocol assumes the presence of a registrar who processes the personal data of participants, and an accountant who processes anonymous ballots. The protocol is protected from collusion between the registrar and the accountant, so we implemented them on one physical server. However, they are logically separated in the code, so you can easily implement them on different servers if needed.

At the forst start Registrar and Accountant ganarate key pair and exchange each other by public keys. Also clients anonimously get this public keys during first connecting to server by onion address. Tor provides routing and end-to-end encryption with the onion-address specified in the invitation, so the client can be sure of the authenticity of the received public keys.

The voting process includes user authentication with a one-time password, personal registration, anonymous gets the ballot, secret vote with server's confirmation and disclosure of vote. Server can be run on such modes: registration (some time before vote day), voting (during vote day) and opening (some time after vote day). By default, the server works in all modes without restriction. Add a sequence of characters as a command line argument to allow only the specified modes:
r for registration;
j for joining (get ballot)
v for vote
o for open
w for activate web interface
So before vote you can run with 'vote.exe rw', during voting day - 'vote.exe jvw' and after voting daty - 'vote.exe ow' commands.

All transactions with the server are confirmed by tickets (server signatures) which the user keeps until the end of the vote. User can use them to prove that the server is fraudulent.
A web interface is provided that allows voters and activists to control the elections. The registrar provides the personal data of the participants and the fact of their registration, the accountant - the numbers of the issued ballots, the fact of their voting and the actual vote after its disclosure.

The server is accessed by onion address via the DarkNet (using Tor Onion anonimizer), providing network anonymity for users.
Tor is integrated both ino server and clients.

Server is implemented on ANSI C and and does not use external dependencies:  all necessary algorithms are implemented in the form of source codes included in the project.  Code is crossplatform (all Windows from 32 bit XP to 64 bit Win10 and mostly all Unix).
Build was tested with gcc under Ubiuntu 16.04 LTS, with mingw-64 under Windows XP and with old Borland C++ Builder.
Executables are tested on Ubuntu 18.04 LTS, Window XP 32, Windows 7 32 and 64, Windows 10. For build use:
make clean
make

Executables is portable and do not require installation on Windows, runs with work folder
Long paths and paths containing cyrillic should be avoided. The best choice is to locate the working folder in the root directory of the D drive. 

Tor is used as prebuided binary plased into Tor folder into work folder. Windows Tor is acceptable for all versions of Windows. On Linux you can install Tor: sudo apt-get install tor
Then copy tor binary from /usr/bin into your work directory. Alternatively you can build Tor statically from last source, see:

For build Tor statically see: 
https://github.com/arlolra/tor/blob/master/INSTALL
https://github.com/cretz/tor-static

Author: Viktoria Malevanchenko, student of NMTU named acad.Yu.Bugai, Ukraine, Poltava, 2020
MaitTo: vika_nmtu@protonmail.com
