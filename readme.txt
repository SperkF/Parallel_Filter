

Program wurde nicht exakt nach Vorgagen implementiert, da die Zeit knapp wurde.
	- Program unterstützt nur P3 Format (Test Images in zip).

Weitere Punkte in denen sich mein Progam von den Vorgangen unterscheidet, von denen ich aber denke das sie gleichwertig sind.
	- worker terminieren nicht mit exit(EXIT_SUCCESS)wenn er merkt das die Message-Queue nicht mehr existiert
	  (ich schicke SIGTERM an Worker wenn ich weiß das ich alle Pixel erhalten habe und terminiere ihn so).
	- Nachdem ich die Worker eben auf meine Methode terminiere, frage ich im status von waitpid() nicht
	  mit WIFEXITED() und WEXITSTATUS() sondern mit WIFSIGNALED() und WTERMSIGNAL() ab ob alles wie erwartet
	  gelaufen ist.
