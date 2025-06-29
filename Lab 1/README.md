# Środowisko pracy

W tym ćwiczeniu zgłębimy kluczowe narzędzia i praktyki niezbędne do efektywnego tworzenia, budowania i debugowania aplikacji w języku C.

## Makefile

Pliki `Makefile` są nieodłącznym elementem procesu kompilacji w środowiskach Unix-like. Pozwalają one na zdefiniowanie zestawu instrukcji, które są potrzebne do budowania aplikacji — włączając w to kompilację źródeł, linkowanie bibliotek oraz zarządzanie zależnościami między plikami.

Podczas tego ćwiczenia dowiemy się, jak tworzyć i zarządzać plikami `Makefile`, co pozwoli na zautomatyzowanie procesu kompilacji i ułatwi pracę nad projektami programistycznymi.

## Środowiska programistyczne

Środowiska programistyczne, zwłaszcza IDE (Integrated Development Environment), są niezastąpionymi narzędziami dla każdego programisty. W ćwiczeniu przyjrzymy się popularnym IDE dla języka C oraz poznamy ich funkcje i możliwości, które pomogą w efektywnym tworzeniu, debugowaniu i testowaniu aplikacji.

## Debugowanie

Techniki debugowania są nieodzowne w procesie rozwoju oprogramowania, zwłaszcza gdy napotykamy na błędy i trudności w naszym kodzie. W trakcie tego ćwiczenia zgłębimy różne metody debugowania w języku C, w tym:

- korzystanie z debuggera (`gdb` lub `lldb`)
- wstawianie punktów kontrolnych (breakpointów)
- analizowanie zmiennych działającego programu

---

# Zadanie

1. Zapoznaj się z koncepcją plików Makefile:  📖 [GNU Make - Introduction](https://www.gnu.org/software/make/manual/html_node/Introduction.html)

2. Zainstaluj/skonfiguruj IDE, w którym będziesz pracować przez resztę laboratoriów (np. VS Code, Vim, etc.).

3. Stwórz nowy projekt w IDE.

4. Napisz prosty program `countdown.c`, który będzie w pętli odliczał od 10 do 0 i wypisywał aktualną liczbę na konsolę (każda liczba w nowej linii).

5. Stwórz plik `Makefile`, za pomocą którego skompilujesz swój program.

6. `Makefile` powinien zawierać co najmniej trzy targety:
    - `all` — buduje wszystkie targety (na razie tylko `countdown`)
    - `countdown` — buduje program `countdown.c`
    - `clean` — usuwa wszystkie pliki binarne, czyści stan projektu

7. W `Makefile`:
    - Użyj zmiennych `CC` oraz `CFLAGS` do zdefiniowania kompilatora (`gcc`) i flag kompilacji (`-Wall`, `-std=c17`, ...).
    - Dodaj specjalny target `.PHONY`.

8. Skompiluj i uruchom program.

9. Korzystając z `gdb` (lub `lldb`), zademonstruj poniższe:
    - zatrzymywanie programu (breakpoint) wewnątrz pętli
    - podejrzenie aktualnego indeksu pętli
    - kontynuacja wykonywania programu
    - wypisanie kolejnego indeksu
    - usunięcie breakpointa
    - kontynuowanie działania programu do końca

   🔗 Dokumentacja:
   - [GDB Manual](https://sourceware.org/gdb/current/onlinedocs/gdb.html/)
   - [GDB Sample Session](https://sourceware.org/gdb/current/onlinedocs/gdb.html/Sample-Session.html#Sample-Session)
   - [LLDB Tutorial](https://lldb.llvm.org/use/tutorial.html)

10. Skonfiguruj swoje IDE do współpracy z `Makefile`.  
    Postaw breakpoint (graficznie, klikając) w środku pętli. Uruchom program w trybie debugowania i zademonstruj wszystkie podpunkty z punktu 9.
