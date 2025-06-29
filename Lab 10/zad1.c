#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>

#define MAX_PATIENTS_IN_HOSPITAL 3
#define MEDICINE_CAPACITY 6
#define MEDICINE_PER_CONSULTATION 3
#define DELIVERY_AMOUNT 3

int patients_in_hospital = 0;
int medicine_in_stock = MEDICINE_CAPACITY;
int patients_remaining;
int pharmacists_remaining;
int patients_waiting[MAX_PATIENTS_IN_HOSPITAL];
bool pharmacist_waiting = false;
int waiting_pharmacist_id = 0;

pthread_mutex_t hospital_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t doctor_sleep_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t patients_cond = PTHREAD_COND_INITIALIZER;
pthread_cond_t pharmacists_cond = PTHREAD_COND_INITIALIZER;

bool doctor_sleeping = true;

void print_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    printf("[%ld.%03ld] ", ts.tv_sec, ts.tv_nsec / 1000000);
}

void* doctor_thread(void* arg) {
    while (patients_remaining > 0 || patients_in_hospital > 0) {
        pthread_mutex_lock(&hospital_mutex);
        
        while (doctor_sleeping && 
               !((patients_in_hospital >= 3 && medicine_in_stock >= MEDICINE_PER_CONSULTATION) ||
                 (pharmacist_waiting && medicine_in_stock < MEDICINE_PER_CONSULTATION))) {
            if (patients_remaining <= 0 && patients_in_hospital == 0) {
                pthread_mutex_unlock(&hospital_mutex);
                return NULL;
            }
            pthread_cond_wait(&doctor_sleep_cond, &hospital_mutex);
        }
        
        if (patients_remaining <= 0 && patients_in_hospital == 0) {
            pthread_mutex_unlock(&hospital_mutex);
            break;
        }
        
        print_time();
        printf("Lekarz: budzę się\n");
        doctor_sleeping = false;
        
        if (patients_in_hospital >= 3 && medicine_in_stock >= MEDICINE_PER_CONSULTATION) {
            print_time();
            printf("Lekarz: konsultuję pacjentów %d, %d, %d\n", 
                   patients_waiting[0], patients_waiting[1], patients_waiting[2]);
            
            pthread_mutex_unlock(&hospital_mutex);
            int consultation_time = 2 + rand() % 3;
            sleep(consultation_time);
            pthread_mutex_lock(&hospital_mutex);
            
            medicine_in_stock -= MEDICINE_PER_CONSULTATION;
            patients_in_hospital = 0;
            
            pthread_cond_broadcast(&patients_cond);
        } 

        else if (pharmacist_waiting && medicine_in_stock < MEDICINE_PER_CONSULTATION) {
            print_time();
            printf("Lekarz: przyjmuję dostawę leków\n");
            
            pthread_mutex_unlock(&hospital_mutex);
            int delivery_time = 1 + rand() % 3;
            sleep(delivery_time);
            pthread_mutex_lock(&hospital_mutex);
            
            medicine_in_stock += DELIVERY_AMOUNT;
            if (medicine_in_stock > MEDICINE_CAPACITY) {
                medicine_in_stock = MEDICINE_CAPACITY;
            }
            
            pharmacist_waiting = false;
            pthread_cond_broadcast(&pharmacists_cond);
        }
        
        print_time();
        printf("Lekarz: zasypiam\n");
        doctor_sleeping = true;
        pthread_mutex_unlock(&hospital_mutex);
    }
    return NULL;
}

void* patient_thread(void* arg) {
    int patient_id = *(int*)arg;
    free(arg);
    
    bool treated = false;
    
    while (!treated && patients_remaining > 0) {
        int arrival_time = 2 + rand() % 4;
        sleep(arrival_time);
        
        print_time();
        printf("Pacjent(%d): Ide do szpitala, bede za %d s\n", patient_id, arrival_time);
        
        pthread_mutex_lock(&hospital_mutex);
        
        if (patients_remaining <= 0) {
            pthread_mutex_unlock(&hospital_mutex);
            break;
        }
        
        if (patients_in_hospital >= MAX_PATIENTS_IN_HOSPITAL) {
            print_time();
            printf("Pacjent(%d): za dużo pacjentów, wracam później za %d s\n", patient_id, arrival_time);
            pthread_mutex_unlock(&hospital_mutex);
            continue;
        }
        
        patients_waiting[patients_in_hospital] = patient_id;
        patients_in_hospital++;
        print_time();
        printf("Pacjent(%d): czeka %d pacjentów na lekarza\n", patient_id, patients_in_hospital);
        
        if (patients_in_hospital == 3 && medicine_in_stock >= MEDICINE_PER_CONSULTATION) {
            print_time();
            printf("Pacjent(%d): budzę lekarza\n", patient_id);
            doctor_sleeping = false;
            pthread_cond_signal(&doctor_sleep_cond);
        }
        
        int my_position = patients_in_hospital - 1;
        while (patients_in_hospital > 0 && 
               (my_position >= 3 || patients_in_hospital < 3 || medicine_in_stock < MEDICINE_PER_CONSULTATION)) {
            if (patients_remaining <= 0) {
                break;
            }
            pthread_cond_wait(&patients_cond, &hospital_mutex);
            
            bool still_waiting = false;
            for (int i = 0; i < patients_in_hospital; i++) {
                if (patients_waiting[i] == patient_id) {
                    my_position = i;
                    still_waiting = true;
                    break;
                }
            }
            if (!still_waiting) {
                treated = true;
                break;
            }
        }
        
        if (my_position < 3 && patients_in_hospital == 0) {
            treated = true;
            patients_remaining--;
        }
        
        pthread_mutex_unlock(&hospital_mutex);
        
        if (treated) {
            print_time();
            printf("Pacjent(%d): kończę wizytę\n", patient_id);
        }
    }
    
    return NULL;
}

void* pharmacist_thread(void* arg) {
    int pharmacist_id = *(int*)arg;
    free(arg);
    
    while (patients_remaining > 0 || patients_in_hospital > 0) {
        int arrival_time = 5 + rand() % 11;
        sleep(arrival_time);
        
        print_time();
        printf("Farmaceuta(%d): ide do szpitala, bede za %d s\n", pharmacist_id, arrival_time);
        
        pthread_mutex_lock(&hospital_mutex);
        
        if (patients_remaining <= 0 && patients_in_hospital == 0) {
            pthread_mutex_unlock(&hospital_mutex);
            break;
        }
        
        if (medicine_in_stock >= MEDICINE_PER_CONSULTATION) {
            print_time();
            printf("Farmaceuta(%d): czekam na oproznienie apteczki\n", pharmacist_id);
            while (medicine_in_stock >= MEDICINE_PER_CONSULTATION && 
                   (patients_remaining > 0 || patients_in_hospital > 0)) {
                pthread_cond_wait(&pharmacists_cond, &hospital_mutex);
            }
            
            if (patients_remaining <= 0 && patients_in_hospital == 0) {
                pthread_mutex_unlock(&hospital_mutex);
                break;
            }
        }
        
        if (medicine_in_stock < MEDICINE_PER_CONSULTATION) {
            print_time();
            printf("Farmaceuta(%d): budzę lekarza\n", pharmacist_id);
            
            pharmacist_waiting = true;
            waiting_pharmacist_id = pharmacist_id;
            doctor_sleeping = false;
            pthread_cond_signal(&doctor_sleep_cond);
            
            while (pharmacist_waiting) {
                pthread_cond_wait(&pharmacists_cond, &hospital_mutex);
            }
            
            print_time();
            printf("Farmaceuta(%d): dostarczam leki\n", pharmacist_id);
        }
        
        print_time();
        printf("Farmaceuta(%d): zakończyłem dostawę\n", pharmacist_id);
        pthread_mutex_unlock(&hospital_mutex);
    }
    
    return NULL;
}

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Użycie: %s <liczba_pacjentów> <liczba_farmaceutów>\n", argv[0]);
        return 1;
    }
    
    int num_patients = atoi(argv[1]);
    int num_pharmacists = atoi(argv[2]);
    
    patients_remaining = num_patients;
    pharmacists_remaining = num_pharmacists;
    
    for (int i = 0; i < MAX_PATIENTS_IN_HOSPITAL; i++) {
        patients_waiting[i] = 0;
    }
    
    srand(time(NULL));
    
    pthread_t doctor;
    pthread_create(&doctor, NULL, doctor_thread, NULL);
    
    pthread_t patients[num_patients];
    for (int i = 0; i < num_patients; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&patients[i], NULL, patient_thread, id);
    }
    
    pthread_t pharmacists[num_pharmacists];
    for (int i = 0; i < num_pharmacists; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&pharmacists[i], NULL, pharmacist_thread, id);
    }
    
    for (int i = 0; i < num_patients; i++) {
        pthread_join(patients[i], NULL);
    }
    
    pthread_mutex_lock(&hospital_mutex);
    doctor_sleeping = false;
    pthread_cond_signal(&doctor_sleep_cond);
    pthread_mutex_unlock(&hospital_mutex);
    
    pthread_join(doctor, NULL);
    
    pthread_mutex_lock(&hospital_mutex);
    pthread_cond_broadcast(&pharmacists_cond);
    pthread_mutex_unlock(&hospital_mutex);
    
    for (int i = 0; i < num_pharmacists; i++) {
        pthread_join(pharmacists[i], NULL);
    }
    
    printf("Wszyscy pacjenci zostali obsłużeni. Koniec symulacji.\n");
    
    pthread_mutex_destroy(&hospital_mutex);
    pthread_cond_destroy(&doctor_sleep_cond);
    pthread_cond_destroy(&patients_cond);
    pthread_cond_destroy(&pharmacists_cond);
    
    return 0;
}