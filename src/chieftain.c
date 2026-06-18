#include <stdlib.h>
#include "config.h"
#include "chieftain.h"
#include "valhalla.h"

void chieftain_init(chieftain_t *self, valhalla_t *valhalla)
{
    self->valhalla = valhalla;
    self->mesa = (int *)malloc(sizeof(int) * config.table_size);
    self->pratos = (int *)malloc(sizeof(int) * config.table_size);
    self->prato1_da_cadeira = (int *)malloc(sizeof(int) * config.table_size);
    self->prato2_da_cadeira = (int *)malloc(sizeof(int) * config.table_size);
    
    for (int i = 0; i < config.table_size; i++) {
        self->mesa[i] = 0; // 0 = livre
        self->pratos[i] = 0; // 0 = livre
        self->prato1_da_cadeira[i] = -1; // -1 indica que não há prato associado
        self->prato2_da_cadeira[i] = -1; // -1 indica que não há prato associado
    }
    
    pthread_mutex_init(&self->mesa_mutex, NULL);
    
    self->vikings_esperando = 0;
    sem_init(&self->fila_espera, 0, 0);
    
    plog("[chieftain] Initialized\n");

}

int chieftain_acquire_seat_plates(chieftain_t *self, int berserker)
{
    while(1){
        int achou_lugar = 0;
        int cadeira = -1;
        pthread_mutex_lock(&self->mesa_mutex);

        for (int i = 0; i < config.table_size; i++) {
            if (self->mesa[i] == 0){
                int seguro = 1;
                if (!berserker){
                    if (i > 0 && self->mesa[i-1] == 2) {
                        seguro = 0;
                    }
                    if (i < config.table_size - 1 && self->mesa[i+1] == 2) {
                        seguro = 0;
                    }
                    
                }else{
                    if (i > 0 && self->mesa[i-1] == 1) {
                        seguro = 0;
                    }
                    if (i < config.table_size - 1 && self->mesa[i+1] == 1) {
                        seguro = 0;
                    }
                }
                int prato_esq = (i - 1 + config.table_size) % config.table_size;
                int prato_meio = i;
                int prato_dir = (i + 1) % config.table_size;

                int pratos_livres = 0;
                if (self->pratos[prato_esq] == 0) pratos_livres++;
                if (self->pratos[prato_meio] == 0) pratos_livres++;
                if (self->pratos[prato_dir] == 0) pratos_livres++;
                if (seguro && pratos_livres >= 2) {
                    self->mesa[i] = berserker ? 2 : 1; // 1 = normal, 2 = berserker

                    cadeira = i;
                    achou_lugar = 1;
                    
                    int p1 = -1, p2 = -1;
                    if (self->pratos[prato_esq] == 0) {
                        p1 = prato_esq;
                    }
                    if (self->pratos[prato_meio] == 0) {
                        if (p1 == -1) {
                            p1 = prato_meio;
                        } else {
                            p2 = prato_meio;
                        }
                    }
                    if (self->pratos[prato_dir] == 0) {
                        if (p1 == -1) {
                            p1 = prato_dir;
                        } else{
                            p2 = prato_dir;
                        }
                    }
                    self->pratos[p1] = 1; // 1 = ocupado
                    self->pratos[p2] = 1; // 1 = ocupado
                    self->prato1_da_cadeira[cadeira] = p1;
                    self->prato2_da_cadeira[cadeira] = p2;
                    break;
                }
            }
        }
        if (achou_lugar){
            pthread_mutex_unlock(&self->mesa_mutex);
            return cadeira;
        }else{
            self->vikings_esperando++;
            pthread_mutex_unlock(&self->mesa_mutex);
            sem_wait(&self->fila_espera);
        }
    }
}

void chieftain_release_seat_plates(chieftain_t *self, int pos)
{
    pthread_mutex_lock(&self->mesa_mutex);
    self->mesa[pos] = 0; // Marca a cadeira como livre
    int prato1 = self->prato1_da_cadeira[pos];
    int prato2 = self->prato2_da_cadeira[pos];
    if (prato1 != -1) {
        self->pratos[prato1] = 0; // Marca o prato como livre
    }
    if (prato2 != -1) {
        self->pratos[prato2] = 0; // Marca o prato como livre
    }
    self->prato1_da_cadeira[pos] = -1;
    self->prato2_da_cadeira[pos] = -1;
    if (self->vikings_esperando > 0) {
        self->vikings_esperando--;
        sem_post(&self->fila_espera);
    }
    pthread_mutex_unlock(&self->mesa_mutex);

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
