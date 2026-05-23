Am folosit Gemini Pro 3.1 pentru implementarea functiilor ajutatoare pentru functia filter.

Prompt 1
"Scrie o functie în C cu semnatura int parse_condition(const char *input, char *field, char *op, char *value) care primeste un string constant de forma 'field:operator:value' si il sparge în 3 buffere separate. nu folosi strtok deoarece nu vreau să modific string-ul original"

Prompt 2 
"Scrie o functie in C cu semnatura int match_condition(Report *r, const char *field, const char *op, const char *value) care primește un pointer la o structura binara report si cele 3 stringuri extrase anterior. Structura contine int severity  time_t timestamp si stringuri pentru category si inspector_name. Functia trebuie sa aplice operatorul (ex: >=, ==) si sa returneze 1 daca e o potrivire."

Output initial
Problemele pe care le-am observat initial
1.In match_condition, AI-ul incerca initial sa compare stringul value direct cu valorile din structura
(ulterior am vazut ca asta e mentionat la sectiunea de hints din document :D )
2.Codul generat folosea strict operatorii matematici. Acest lucru facea ca rularea din terminal (severity:>=:2) sa fie interpretata de shell ca o redirectionare de output in fisier


Pentru a corecta aceste aspecte, am ghidat AI-ul cu următoarele prompturi:

Prompt 3
"Ai gresit la conversia tipurilor. Modifica match_condition astfel incat sa converteasca value la int pentru severity si la long long pentru timestamp , inainte de a face comparatia."

Prompt 4
"Cand filtrul are caracterul >, se creeaza  un fisier si redirecteaza output acolo in loc sa fie trimis ca argument. Adapteaza match_condition ca sa accepte acesti operatori."

Pe parcursul proiectului, am folosit Visual Studio Code, cu Intellisense Code Completion (nu stiu daca intra la categoria de AI, mentionez in orice eventualitate).

Am invatat ca ai AI tinde sa se concentreze strict pe limbajul cerut (C), izoland de mediul de executie , de asta nu a anticipat problema redirectionarilor.


PHASE 2
Am folosit AI pentru explicarea catorva concepte, sau anumite clarificari.

Functionalitatea apelurilor fork si exec.

Am intrebat cum se poate interoga nucleul sistemului de operare pentru a verifica daca PID-ul unui vechi monitor mai este activ, fara a trimite un semnal distructiv.

Am intrebat cum se face trecerea de la un descriptor de fisier returnat de pipe() la un flux de date de nivel inalt (FILE*) folosind fdopen. Aceasta tehnica ne-a permis sa utilizam functia fgets pentru a citi mesaje text linie cu linie din conducta, beneficiind de parsarea automata a caracterului newline, fara a mai fi nevoie sa scriem un parser manual de octeti peste un apel read clasic.

PHASE 3

La fel ca la phase 2, am folosit AI pentru explicatii 

Am intrebat de ce dimensiunea pe disc pentru doua rapoarte era de exact 528 bytes, desi suma campurilor din structura mea(city_commands.h) era de 256 bytes. Raspunsul a fost ca compilatorul gcc insereaza 8 bytes de umplutura pentru alinierea campului timestamp (time_t) pe arhitecturi de 64 de biti, structura avand de fapt 264 bytes.

Am clarificat modul in care dup2 redirectioneaza iesirea standard a procesului scorer catre capatul de scriere al pipe-ului si importanta inchiderii capetelor nefolosite in procesul parinte pentru a evita blocarea apelurilor de citire.

Am discutat implementarea corecta a apelului waitpid pentru a curata resursele proceselor copil imediat dupa ce acestea isi incheie executia si transmit datele

Am avut o problema cu disparitia promptului din city_hub, cand monitorul trimitea o alerta in timp real mesajul aparea brusc pe ecran si stergea vizual promptul din terminal.
Raspunsul LLM-ului a fost adaugarea unui printf("hub> ") si a unui fflush(stdout) imediat dupa afisarea alertei, doar in interiorul ramurii de cod care proceseaza notificarea respectiva


===========================================

Pe parcursul intregului proiect, am folosit LLM pentru a genera date de test, cu instructiunea de a genera, pe langa date generale si edge case-uri pentru a testa pe indelete noile
proprietati pe parcurs ce erau adaugate.
