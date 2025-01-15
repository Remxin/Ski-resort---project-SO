#ifndef CONFIG_H
#define CONFIG_H

// Definicje zmiennych i parametrów konfiguracyjnych

#define MAX_SKIERS 5
#define HAS_CHILDREN_PROBABILITY 0.3

// Szansa, że ktoś zostanie VIP-em
#define VIP_CHANCE 0.15  // szansa, że ktoś będzie VIP

#define MAX_AGE 80  // zmienna potrzebna do generowania narciarzy
// Wiek, powyżej którego osoba jest uznawana za seniora
#define SENIOR_AGE 65

// Wiek, poniżej którego osoba jest dzieckiem
#define CHILD_AGE 12
#define MAX_CHILDREN 4

// Wiek, poniżej którego dzieci muszą być pod opieką dorosłego
#define YOUNG_CHILD_AGE 8

// Liczba osób, które mogą przebywać na peronie dolnej stacji
#define MAX_PEOPLE_AT_STATION 100

// Inne parametry, jak np. czas przejazdu dla różnych tras (T1, T2, T3)
#define T1 10  // Czas przejazdu na trasie 1 (w sekundach)
#define T2 15  // Czas przejazdu na trasie 2 (w sekundach)
#define T3 20  // Czas przejazdu na trasie 3 (w sekundach)

#endif
