---   PROTOCOALE DE COMUNICATIE   ---
                Tema 2
  Aplicatie client-server TCP si UDP
     pentru gestionarea mesajelor

    321 CC - Pavel Calin Gabriel  

struct.h
    -> User - sunt retinute datele fiecarui 
    client TCP, acesta are socketul curent , 
    starea de conectare si un vector de structuri
     de topicuri.

    -> Topic - se retine numele topicului
    pentru identificare , flag-ul de SF 
    ce permite redirectarea datagramelor din
    storage . Memoria de storage este folosita
    pentru a stoca datele primite de clientii
    UDP in momentul in care clientul TCP , abonat
    la respectivul topic , nu este conectat.

    ->UdpPacket - aceasta structura am folosit-o
    pentru parsarea si manipularea datagramelor
    primite in urma comunicarii cu clientii UDP.

    ->TcpPacket - aceasta structura am folosit-o
    pentru crearea pachetelor de tip TCP si transmiterea
    acestora la clientii corespunzatori


subscriber.cpp
    -> sursa ce asigura comunicarea dintre clientii TCP si
    server
    -> verifica prezenta celor 3 parametrii corespunzatori
    conectarii la server , restul comenzilor sunt verificate
    in cadrul server-ului
    -> a fost necesara transmiterea independenta id-ul de logarea
    in scopul folosirii acestuia in procesul de identidicare a
    user-ului in server
    -> comenzile de subscribe/unsubscribe sunt tratate in cadrul serverului
    -> am dezactivat algoritmulul lui Neagle(sursa: adaugata in server.cpp)

server.cpp
    -> functiile de init au rolul de a organiza lucrul cu baza de date
    -> recv_from_udp - se parseaza datagrama primita din partea clientilor
    UDP prin intermediul structurii UdpPacket , se identifica tipul de data
    primit si se trateaza in functie de caz
                     - se realizeaza cast-ul datelor si se construieste 
    pachetul necesar trasmiterii TCP
                     - se verifica listele de abonare ale userilor , apoi
    starea de conexiune intre user si server si in ultimul rand starea flag-
    ului de SF  
                     - in cazul in care este deconectat in timpul sesiunii Udp
    datagramele corespunzatoare topicului de abonare se pun in storage pentru
    trimiterea ulterioara
                     - totodata se verifica cazul in care un client conectat
    are date in storage si flag-ul de SF activ , pentru transmiterea datagramelor
    corespunzatoare
                    - SF-ul functioneaza doar in cazul in care catre clientul 
    reconectat trebuie transmisa cel putin o datagrama noua
    ->instructions - in aceasta functie se trateaza instructiunile de subscribe/
    unsubscribe
                    -in cazul instructiunii de subscribe se verifica daca numarul
    de parametrii este corect si daca user-ul respectiv nu este deja abonat la 
    topic-ul cerut , la momentul reusirii procesului de abonare se trimite un mesaj
    corespunzator catre user
                    -in cazul instructiunii de unsubscribe se verifica daca topicul
    cerut este gasit in baza de date , la un raspuns afirmativ acesta este inlaturat
    din lista de topicuri ale userului respectiv
    ->functiile de check_id si check_user_id sunt destinate primirii si verificarii
    id-ului de user la momentul stabilirii conexiunii intre server si un client TCP
    ->main -in functia main sunt tratate cele doua posibile tipuri de conexiuni TCP/
    UDP,realizand redirectarea necesara in functie de socket si starea de conectare
           -in cazul conectarii TCP se verifica deaseamena conectarea/deconectarea
    si unicitatea id-ului
           -am dezactivat algoritmul lui Neagle prin intermediul instructiunii 
    TCP_NODELAY(am folosit sursa pentru intelegere
    :https://stackoverflow.com/questions/17842406/how-would-one-disable-nagles-algorithm-in-linux)

    




