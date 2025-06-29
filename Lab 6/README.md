# Potoki

## Zadanie 1

Napisz program, który **liczy numerycznie wartość całki oznaczonej** z funkcji:

`4 / (x² + 1)`

w przedziale od `0` do `1`, **metodą prostokątów** (z definicji całki oznaczonej Riemanna).

- Pierwszy parametr programu to **szerokość każdego prostokąta**, określająca dokładność obliczeń.
- Obliczenia należy rozdzielić na **k procesów potomnych**, tak by każdy z procesów liczył inny fragment ustalonego wyżej przedziału.
- Każdy z procesów powinien wynik swojej części obliczeń przesłać **przez potok nienazwany** do procesu macierzystego.
- **Każdy proces potomny** do komunikacji z procesem macierzystym powinien używać **osobnego potoku**.
- Proces macierzysty powinien oczekiwać na wyniki uzyskane od wszystkich procesów potomnych, po czym dodać te wyniki cząstkowe i:
  - wyświetlić wynik na standardowym wyjściu,
  - podać czas wykonania oraz odpowiadającą mu wartość `k`.

Program powinien przeprowadzić powyższe obliczenia dla wartości:

`k = 1, 2, ..., n`  
gdzie `n` to **drugi parametr wywołania programu**.

W wyniku działania programu, na standardowym wyjściu powinny zostać wypisane:

- wyniki obliczeń,
- czasy wykonania tych obliczeń,
- dla każdej liczby użytych procesów od `1` do `n`.

**Uwaga:**  
Dobierz dokładność obliczeń tak, by trwały one co najmniej kilka sekund.
