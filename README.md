
# Zadanie 'Egzekutor'

Sara rozwija pewien algorytm i pisze programy, które wykonują się długo, wypisując co jakiś czas informacje o postępach. Sara chciałaby testować swoje programy, wykonując je na różnych przykładach i sprawdzając co wypisują. Przykładów do sprawdzenia i wypisywanych informacji jest jednak bardzo dużo. Przygotuj dla Sary program `initExecutor`, który w tym pomoże, wykonując wskazane zadania, sprawdzając ich ostatnie słowa (w trakcie działania programu) i ubijając je.

Egzekutor (program `initExecutor`) powinien obsługiwać następujące polecenia (każde polecenie to jedna linia wejścia):

### Polecenie run

Polecenie `run A B C ...` tworzy nowe zadanie, rozpoczynając wykonanie programu `A` z argumentami `B C ...` w tle. Zadania identyfikowane są liczbami 0, 1, 2, ..., w kolejności rozpoczęcia (tzn. poleceń `run`). Egzekutor wypisuje `Task T started: pid P.\n`, gdzie `T` to identyfikator zadania, zaś `P` to pid procesu wykonującego program `A`. Cokolwiek program `A` wypisze na standardowe wyjście i wyjście błędów nie powinno się pokazywać. Program `A` nigdy nie będzie oczekiwać standardowego wejścia (można je zostawić bez zmian).

### Polecenie out

Polecenie `out T` wypisuje `Task T stdout: 'S'.\n`, gdzie `S` to ostatnia linia (bez znaku końca linii) dotychczas wypisana przez program `A` z zadania nr `T`. W rozwiązaniu na mniej punktów, egzekutor może zaczekać, aż program `A` wypisze kolejną linię (lub `EOF`) i użyć jej jako `S`. W rozwiązaniu na pełną liczbę punktów, egzekutor powinien niezwłocznie wypisać ostatnią linię (o ile program `A` przez dłuższy czas nic więcej nie wypisze; jeśli program `A` w krótkim czasie wypisze kolejne linie, można użyć też dowolnej z nich). Program `A` powinien kontynuować działanie.

Jeśli program jeszcze nic nie wypisał, można użyć pustego `S`. Jeśli program się zakończył, `S` powinno być ostatnią linią wypisaną przez cały program.

### Polecenie err

Polecenie `err T` wypisuje `Task T stderr: 'S'.\n`, analogicznie jak out `T`, ale dla standardowego wyjścia błędów programu `A`. Wypisuje to na zwykłe standardowe wyjście egzekutora (tak jak wszystkie polecenia egzekutora).

### Polecenie kill

Polecenie `kill T` wysyła sygnał `SIGINT` do programu `A` z zadania nr `T`. Jeśli program już się zakończył, można wysłać sygnał (nieskutecznie, ignorując błąd) lub nie robić nic.

### Polecenia pomocnicze

- `sleep N` – po prostu usypia egzekutora na `N` milisekund (używając np. [usleep](https://linux.die.net/man/3/usleep), który używa mikrosekund), gdzie `N` to liczba całkowita. Wstrzymuje na ten czas przetwarzanie kolejnych poleceń; nie wstrzymując żadnych zadań.
- `quit` lub koniec wejścia (`EOF`) – kończy egzekutor (i wszystkie programy).
- pusta linia – nic nie robi, przechodzi do kolejnego polecenia.

### Kończenie zadań

Gdy program w zadaniu nr `T` się zakończy, egzekutor powinien o tym poinformować wypisując `Task T ended: status X.\n`, gdzie `T` to identyfikator zadania, zaś `X` to jego kod wyjścia. Jeśli kod wyjścia nie jest dostępny (bo zadanie zostało przerwane sygnałem lub przez system), należy zamiast tego wypisać `Task T ended: signalled.\n`.

Jeśli program zakończy się w trakcie obsługi jakiegoś polecenia przez egzekutora, informacja powinna się wypisać dopiero po zakończeniu tej obsługi. W rozwiązaniu na mniej punktów, można poczekać z wypisaniem informacji aż do najbliższego polecenia (i jego zakończenia). W rozwiązaniu na pełną liczbę punktów, informacja powinna wypisać się niezwłocznie po zakończeniu programu, czyli podczas czekania egzekutora na kolejne polecenia (o ile nie trwa właśnie obsługa polecenia).

Kiedy egzekutor poinformuje o zakończeniu ostatniego (z dotychczas rozpoczętych) zadań, nie powinien istnieć żaden proces ani wątek stworzony przez egzekutora, poza samym głównym wątkiem egzekutora i ewentualnie jednym pomocniczym wątkiem/procesem.

### Przykład

Wykonanie w bashu

```bash
echo -e "foo\nbar" > in.txt;
echo -e "run cat in.txt\nsleep 100\nout 0" | ./initExecutor
```

powinno wypisać na przykład:

```console
Task 0 started: pid 1234.
Task 0 ended: status 0.
Task 0 stdout: 'bar'.
```

### Uwagi

- Zużycie pamięci egzekutora powinno być niezależne, O(1), od długości wyjścia i czasu działania wykonywanych programów.
- Egzekutor nie może tworzyć zwykłych plików.
- Egzekutor nie powinien zaczynać przyjmować kolejnego polecenia na wejściu do czasu zakończenia obsługi poprzedniego.
- Zakazane jest użycie funkcji `poll()` i wariantów (`ppoll`, `select`, `pselect`, `epoll`).
- Poza funkcjami omawianymi na zajęciach, można użyć też innych funkcji ze standardu `C` lub `POSIX`, z wyjątkiem `poll()`. Żadne nie są konieczne, przydać się mogą [waitpid](https://linux.die.net/man/3/wait) (z opcją `WNOHANG` do sprawdzenia czy ktoś skończył działanie) oraz [fdopen](https://linux.die.net/man/3/fdopen) (żeby móc korzystać z funkcji operujących na strumieniach, np. `getline` czy `fgets`).
- Zakazane jest aktywne oczekiwanie; ściślej: jeśli przez dłuższy czas na wejściu nie pojawiają się kolejne polecenia i programy nic nie wypisują, nie można w tym czasie zużywać CPU. W szczególności nie można okresowo budzić procesu.
- Uruchomione programy nie powinny mieć żadnych otwartych deskryptorów poza standardowymi.

Aby zminimalizować trudności z obsługą pamięci i błędów w języku C:

- Wszystkie polecenia dla egzekutora będą poprawne i nie dłuższe niż 511 znaków (włączając koniec linii).
- Wszystkie linie wypisane przez wykonywane programy będą nie dłuższe 1022 znaki (włączając koniec linii; ostatnia linia może, ale nie musi zawierać znaku końca linii).
- Wszystkich zadań będzie co najwyżej 4096. Egzekutor może zużywać pamięć proporcjonalną do tej liczby, pomnożonej przez maksymalną długość linii, nawet jeśli nie uruchomimy tylu zadań. Na przykład można mieć globalną tablicę `struct Task tasks[MAX_N_TASKS];`.
- Wykonywane programy nigdy nie wypiszą znaku zerowego. Polecenia egzekutora nigdy nie będą zawierać znaku zerowego, cudzysłowów, ani znaku `\`. W szczególności argumenty programów są po prostu odzielone spacjami i można je przetworzyć używając funkcji 		`split_string` z `utils.h`.
- W przypadku dowolnego błędu można po prostu zakończyć działanie egzekutora, np. `exit(1)`.
- Można wypisać dowolne informacje diagnostyczne, w szczególności o błędach, na `stderr`.

### Wymagania techniczne

- Załączony jest szablon rozwiązania `ab1234567.tar`. Można w nim znaleźć pliki `utils.h` i `utils.c` z paroma funkcjami, które mogą być pomocne. Wszystkie pliki można dowolnie modyfikować.
- Rozwiązanie należy wysłać na Moodle w postaci pliku `ab1234567.tar` (lub `.tgz`), zawierającego folder `ab1234567`, zawierającego `CMakeLists.txt`. Oczywiście `ab1234567` należy zmienić na własne inicjały i numer indeksu.
- Rozwiązanie musi działać na maszynie `students`, skompilowane za pomocą `CMake` (polecenie `cd ab1234567 && mkdir build && cd build && cmake .. && make` powinno wyprodukować plik wykonywalny `ab1234567/build/initExecutor`).
- Użycie `clang-format` i `clang-tidy` nie jest wymagane (ale jest jak zwykle zalecane).

#### Pytania

Pytania prosimy zadawać na forum.

