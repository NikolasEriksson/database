# database

Tankar och idéer:
Skapa upp nya filer för CREATE table, ta bort filerna för DROP table.
Lås filen vid INSERT, använd semaphore, 0 och 1 för att säga till om den är låst eller ej, loopa under tiden den är låst tills den inte är låst längre.
Shellscript för att hämta ut vid SELECT, använd Shellscript för alla andra saker, CREATE, DROP och INSERT.

Zombieprocess info: geeksforgeeks har bra info om hur man gör.


Database in C code with telnet client server


#TODO server side
* add semaphores or some way of control flow.
* make sure to close the main socket.
