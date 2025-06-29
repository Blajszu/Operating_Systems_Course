/**
 * W tym programie mamy dwa wątki: producenta i konsumenta. Wątek producenta produkuje 
 * wartości i przechowuje je w współdzielonym buforze, podczas gdy wątek konsumenta 
 * pobiera wartości z bufora.
 * 
 * Używamy muteksu i dwóch zmiennych warunkowych do synchronizacji dostępu do bufora. 
 * Muteks zapewnia, że tylko jeden wątek może jednocześnie uzyskać dostęp do bufora, 
 * podczas gdy zmienne warunkowe pozwalają wątkom sygnalizować sobie nawzajem, gdy bufor 
 * jest pełny lub pusty. 
 * 
 * Wątek producenta zajmuje muteks, sprawdza za pomocą zmiennej count, czy bufor jest 
 * pełny, i jeśli tak, oczekuje na zmiennej warunkowej producer_cond. Gdy w buforze jest 
 * miejsce, produkuje wartość, przechowuje ją w buforze i sygnalizuje zmiennej warunkowej, 
 * aby powiadomić wątek konsumenta. Na koniec zwalnia muteks.
 * 
 * Wątek konsumenta zajmuje muteks, sprawdza za pomocą zmiennej count, czy bufor jest 
 * pusty, i jeśli tak, oczekuje na zmiennej warunkowej consumer_cond. Gdy wartość jest 
 * dostępna w buforze, pobiera ją, przetwarza wartość i sygnalizuje zmiennej warunkowej 
 * producer_cond, aby powiadomić wątek producenta. Na koniec zwalnia muteks.
 * 
 * Funkcja main tworzy wątki producenta i konsumenta, a następnie oczekuje na ich 
 * zakończenie.
*/
#include "checks.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

#define BUFFER_SIZE 10
#define VALUE_COUNT 20

int buffer[BUFFER_SIZE];
int count = 0;

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t producer_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t consumer_cond = PTHREAD_COND_INITIALIZER;

void *producer(void *arg)
{
    /* Podaj stworzony przez siebie, odpowiedni muteks do funkcji check_mutex_producer */
    check_mutex_producer(&mutex);

    /* Podaj stworzoną przez siebie, odpowiednią zmienną warunkową do funkcji check_cond_producer */
    check_cond_producer(&producer_cond);

    int i;
    for (i = 0; i < VALUE_COUNT; ++i)
    {
        /* Zajmij mutex */
        pthread_mutex_lock(&mutex);

        /* Tu czekaj tak długo jak: count == BUFFER_SIZE */
        while(count == BUFFER_SIZE) {
            pthread_cond_wait(&producer_cond, &mutex);
        }

        buffer[count++] = i;

        /* Zasygnalizuj, że bufor jest pełny */
        pthread_cond_signal(&consumer_cond);

        /* Zwolnij mutex */
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

void *consumer(void *arg)
{
    /* Podaj stworzony przez siebie, odpowiedni muteks do funkcji check_mutex_consumer */
    check_mutex_consumer(&mutex);

    /* Podaj stworzoną przez siebie, odpowiednią zmienną warunkową do funkcji check_cond_consumer */
    check_cond_consumer(&consumer_cond);

    int i;
    for (i = 0; i < VALUE_COUNT; ++i)
    {
        /* Zajmij mutex */
        pthread_mutex_lock(&mutex);

        /* Tu czekaj tak długo jak: count == 0 */
        while(count == 0) {
            pthread_cond_wait(&consumer_cond, &mutex);
        }

        int value = buffer[--count];
        process_value(value);

        /* Zasygnalizuj, że bufor jest pusty */
        pthread_cond_signal(&producer_cond);

        /* Zwolnij mutex */
        pthread_mutex_unlock(&mutex);
    }

    return NULL;
}

int main()
{
    /* Stwórz wątek producenta, podaj mu funkcję producer() do wykonywania */ 
    pthread_t producer_thread;
    if(pthread_create(&producer_thread, NULL, producer, NULL) != 0) {
        fprintf(stderr, "Błąd podczas tworzenia wątku producenta");
        return 1;
    }

    /* Stwórz wątek konsumenta, podaj mu funkcję consumer() do wykonywania */
    pthread_t consumer_thread;
    if(pthread_create(&consumer_thread, NULL, consumer, NULL) != 0) {
        fprintf(stderr, "Błąd podczas tworzenia wątku konsumenta");
        return 1;
    }

    /* Poczekaj na zakończenie działania wątków */
    pthread_join(producer_thread, NULL);
    pthread_join(consumer_thread, NULL);

    check_wait();
    check_results();

    return 0;
}
