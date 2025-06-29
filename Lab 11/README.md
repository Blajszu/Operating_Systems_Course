# Sockety

## Prosty chat

Napisz prosty chat typu klient-serwer w którym komunikacja pomiędzy uczestnikami czatu / klientami / klientami i serwerem realizowana jest za pośrednictwem socketów z użyciem protokołu strumieniowego.

### Parametry uruchomienia:
- **Serwer**:  
  Adres/port serwera podawany jest jako argument jego uruchomienia

- **Klient**:  
  Przyjmuje jako argumenty:
  1. swoją nazwę/identyfikator (string o z góry ograniczonej długości)
  2. adres serwera (adres IPv4 i numer portu)

### Protokół komunikacyjny:
1. **LIST**  
   Pobranie z serwera i wylistowanie wszystkich aktywnych klientów

2. **2ALL string**  
   Wysłanie wiadomości do wszystkich pozostałych klientów.  
   - Klient wysyła ciąg znaków do serwera  
   - Serwer rozsyła ten ciąg wraz z:
     * identyfikatorem nadawcy
     * aktualną datą  
   do wszystkich pozostałych klientów

3. **2ONE id_klienta string**  
   Wysłanie wiadomości do konkretnego klienta.  
   - Klient wysyła do serwera ciąg znaków podając jako adresata konkretnego klienta  
   - Serwer wysyła ten ciąg wraz z:
     * identyfikatorem klienta-nadawcy  
     * aktualną datą  
   do wskazanego klienta

4. **STOP**  
   Zgłoszenie zakończenia pracy klienta.  
   Powinno skutkować usunięciem klienta z listy klientów przechowywanej na serwerze

5. **ALIVE**  
   - Serwer powinien cyklicznie "pingować" zarejestrowanych klientów  
   - Jeśli klient nie odpowiada - usuwa go z listy klientów

6. **Obsługa Ctrl+C**  
   Klient przy wyłączeniu Ctrl+C powinien wyrejestrować się z serwera

### Uproszczenia:
Serwer może przechowywać informacje o klientach w statycznej tablicy (rozmiar tablicy ogranicza liczbę klientów, którzy mogą jednocześnie być uczestnikami czatu).
