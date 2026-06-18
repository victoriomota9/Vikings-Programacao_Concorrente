#include <stdlib.h>
#include "config.h"
#include "chieftain.h"
#include "valhalla.h"

void chieftain_init(chieftain_t *self, valhalla_t *valhalla)
{
    /* TODO: Adicionar código aqui se necessário! */
    self->mesa = (int *)malloc(sizeof(int) * config.table_size);
    self->pratos = (int *)malloc(sizeof(int) * config.table_size);
    for (int i = 0; i < config.table_size; i++) {
        self->mesa[i] = 0; // 0 = livre
        self->pratos[i] = 0; // 0 = livre
    }
    pthread_mutex_init(&self->mesa_mutex, NULL);
    
    int max_vikings = config.horde_size * 2;
    self->viking_semaforo = (sem_t *)malloc(sizeof(sem_t) * max_vikings);

    for (int i = 0; i < max_vikings; i++) {
        sem_init(&self->viking_semaforo[i], 0, 0); // Inicializa o semáforo para cada viking
    }

    self->valhalla = valhalla;
    plog("[chieftain] Initialized\n");

}

int chieftain_acquire_seat_plates(chieftain_t *self, int berserker)
{
    /* TODO: Implementar! */
    return 1;
}

void chieftain_release_seat_plates(chieftain_t *self, int pos)
{
    /* TODO: Implementar! */
}

god_t chieftain_get_god(chieftain_t *self)
{
    /* TODO: Implementar! O código abaixo deve ser modificado! */
    god_t god = THOR;
    
    return god;
}

void chieftain_finalize(chieftain_t *self)
{
    /* TODO: Adicionar código aqui se necessário! */

    plog("[chieftain] Finalized\n");
}
