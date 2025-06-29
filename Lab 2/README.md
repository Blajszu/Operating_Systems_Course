# Biblioteki w systemie Unix

Biblioteki są kluczowymi składnikami każdego systemu operacyjnego, a ich zrozumienie jest niezbędne dla skutecznego tworzenia i zarządzania aplikacjami. W ramach tego ćwiczenia przyjrzymy się trzem głównym typom bibliotek: **statycznym**, **współdzielonym** i **dynamicznym**.

## Rodzaje bibliotek

- **Biblioteki statyczne** są skompilowanymi plikami binarnymi, które są dołączane do programów podczas kompilacji. Funkcje zawarte w takiej bibliotece są kopiowane bezpośrednio do pliku wykonywalnego programu.

- **Biblioteki współdzielone** są ładowane do pamięci podczas uruchamiania programu i mogą być współdzielone przez wiele aplikacji, co zmniejsza rozmiar końcowego pliku wykonywalnego.

- **Biblioteki dynamiczne** są podobne do współdzielonych, ale są ładowane i usuwane dynamicznie **w trakcie działania aplikacji** (np. za pomocą `dlopen`/`dlsym`).

---

## Problem Collatza

**Problem Collatza** (Collatz Conjecture), znany również jako problem *3n + 1*, to jedno z najbardziej znanych nierozwiązanych problemów matematycznych.

Reguły:
- Jeśli liczba jest **parzysta**, podziel ją przez 2.
- Jeśli liczba jest **nieparzysta**, pomnóż ją przez 3 i dodaj 1.

Zakłada się, że niezależnie od wartości początkowej, liczba zawsze zbiegnie do 1. Mimo że nie ma formalnego dowodu, problem ten został przetestowany empirycznie dla ogromnej liczby przypadków.

---

# Zadanie

1. **Stwórz bibliotekę w języku C** zawierającą dwie funkcje:

    ```c
    int collatz_conjecture(int input);
    ```

    - Zasada działania:
        - Jeśli `input` jest parzysty → zwróć `input / 2`
        - Jeśli nieparzysty → zwróć `3 * input + 1`

    <br>
    
    ```c
    int test_collatz_convergence(int input, int max_iter, int *steps);
    ```

    - Zasada działania:
        - Iteracyjnie stosuje regułę Collatza.
        - Zapisuje każdy wynik do tablicy `steps`.
        - Kończy działanie, gdy liczba osiągnie 1 lub zostanie przekroczony `max_iter`.
        - Zwraca liczbę wykonanych kroków.
        - Jeśli nie osiągnięto 1 w `max_iter` iteracjach — zwraca `0`.

2. **W `Makefile` utwórz dwa wpisy**:
    - do kompilacji biblioteki statycznej (`.a`)
    - do kompilacji biblioteki współdzielonej (`.so`)

3. **Napisz program-klienta**, który:
    - Testuje kilka różnych liczb.
    - Korzysta z `test_collatz_convergence`.
    - Dla każdej liczby:
        - wypisuje pełną sekwencję redukcji do 1 (jeśli się uda),
        - lub wypisuje komunikat o niepowodzeniu (jeśli przekroczy `max_iter`).

4. **Zaimplementuj klienta w trzech wariantach**:
    - korzystającego z **biblioteki statycznej**
    - korzystającego z **biblioteki współdzielonej**
    - korzystającego z **biblioteki ładowanej dynamicznie** (np. z użyciem `dlopen`, `dlsym`)

5. **Dla każdego z trzech wariantów klienta utwórz odpowiedni wpis w `Makefile`.**

6. **Użyj makra preprocesora (`-D`)** w `Makefile`, aby zmieniać sposób działania klienta — w zależności od tego, z której wersji biblioteki ma korzystać.
