

Program wurde nicht exakt nach Vorgagen implementiert
	- Program unterstützt nur P3 Format (Test Images in zip).
	- worker terminieren nicht mit exit(EXIT_SUCCESS)wenn er merkt das die Message-Queue nicht mehr existiert
	  (ich schicke SIGTERM an Worker wenn ich weiß das ich alle Pixel erhalten habe und terminiere ihn so).
	- Nachdem ich die Worker eben auf meine Methode terminiere, frage ich im status von waitpid() nicht
	  mit WIFEXITED() und WEXITSTATUS() sondern mit WIFSIGNALED() und WTERMSIGNAL() ab ob alles wie erwartet 
	  gelaufen ist.
	- Program verwendet msgsnd() und msgrcv() nur in Non-blocking mode.
	 -> Versuche mit Non-blocking mode haben nicht geklappt, der Transport der Packete
	    über die Message-Queue war unzuverlässig. (es sind immer wieder random pakete "verschwunden")
	 -> habe mich deshalb dazu entschieden ein weiteres mal zu forken und somit folgene Struktur:
	Master
fork()  Master		worker_1	worker_2	...user definierte Anzahl bzw. mit get_nprocs()
fork()	Master(liest packete)		Master_snd(sendet packete)


