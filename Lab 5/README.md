# Sygnały i komunikacja międzyprocesowa

## Rodzaje sygnałów

- `SIGINT`, `SIGQUIT`, `SIGKILL`, `SIGTSTP`, `SIGSTOP`, `SIGTERM`, `SIGSEGV`, `SIGHUP`, `SIGALARM`, `SIGCHLD`, `SIGUSR1`, `SIGUSR2`

## Sygnały czasu rzeczywistego

- `SIGRTMIN`, `SIGRTMIN+n`, `SIGRTMAX`

## Przydatne polecenia Unix

- `kill`, `ps`

## Przydatne funkcje systemowe

- `kill`, `raise`, `sigqueue`, `signal`, `sigaction`, `sigemptyset`, `sigfillset`, `sigaddset`, `sigdelset`, `sigismember`, `sigprocmask`, `sigpending`, `pause`, `sigsuspend`

---

## Zadanie 1

Napisz program demonstrujący różne reakcje na przykładowy sygnał `SIGUSR1` w zależności od ustawionych dyspozycji.  

Reakcja na sygnał `SIGUSR1` programu powinna zależeć od wartości argumentu z linii poleceń.  
Argument ten może przyjmować wartości: `none`, `ignore`, `handler`, `mask`.  

Program w zależności od parametru powinien odpowiednio:

- nie zmieniać reakcji na sygnał (`none`),
- ustawiać ignorowanie (`ignore`),
- instalować handler obsługujący sygnał (działający w ten sposób, że wypisuje komunikat o jego otrzymaniu) (`handler`),
- maskować ten sygnał oraz sprawdzać, czy wiszący/oczekujący sygnał jest widoczny (`mask`).

Następnie przy pomocy funkcji `raise` program wysyła sygnał do samego siebie oraz wykonuje odpowiednie dla danej opcji, opisane wyżej działania.

---

## Zadanie 2

Napisz dwa programy:

- **sender** — program wysyłający sygnały `SIGUSR1`
- **catcher** — program odbierający te sygnały

### catcher

- Uruchamiany jako pierwszy
- Wypisuje swój numer PID i czeka na sygnały `SIGUSR1`
- Po odebraniu `SIGUSR1`, wysyła do **sendera** sygnał `SIGUSR1` jako potwierdzenie
- Oczekiwanie na potwierdzenie może być realizowane z użyciem `sigsuspend`
- Wraz z każdym sygnałem od **sendera**, odbiera także **tryb pracy**, przesyłany za pomocą `sigqueue`
- Możliwe tryby pracy:

  1. Wypisanie na standardowym wyjściu liczby otrzymanych żądań zmiany trybu pracy od początku działania programu
  2. Wypisywanie na standardowym wyjściu kolejnej liczby co jedną sekundę, aż do momentu otrzymania innego trybu pracy
  3. Ustawienie ignorowania reakcji na `Ctrl+C`
  4. Ustawienie reakcji na `Ctrl+C` jako wypisanie tekstu: `"Wciśnięto CTRL+C"`
  5. Zakończenie działania programu

- **PID sendera** catcher pobiera ze struktury `siginfo_t` po odebraniu sygnału

### sender

- Przyjmuje jako argumenty:
  - PID procesu catcher
  - Tryb pracy procesu catcher (jeden tryb na jedno wywołanie)
- Działa do momentu otrzymania potwierdzenia, że catcher odebrał sygnał
- Może być wywoływany wielokrotnie, aż do zakończenia działania catchera (tryb 5)

---

**Uwaga:**  
W żaden sposób nie opóźniamy wysyłania sygnałów — wszelkie "gubienie" sygnałów jest zjawiskiem naturalnym.
