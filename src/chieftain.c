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

    for (int i = 0; i < config.table_size; i++)
    {
        self->mesa[i] = 0;               // 0 = livre
        self->pratos[i] = 0;             // 0 = livre
        self->prato1_da_cadeira[i] = -1; // -1 indica que não há prato associado
        self->prato2_da_cadeira[i] = -1; // -1 indica que não há prato associado
    }
    for (int i = 0; i < NUMBER_OF_GODS; i++) {
        self->preces_autorizadas[i] = 0;
    }

    pthread_mutex_init(&self->mesa_mutex, NULL);
    pthread_mutex_init(&self->autorizacao_de_rezar_mutex, NULL);

    self->vikings_esperando = 0;
    sem_init(&self->fila_espera, 0, 0);

    self->vikings_que_ja_comeram = 0;
    sem_init(&self->preces_barrier, 0, 0);

    plog("[chieftain] Initialized\n");
}

int chieftain_acquire_seat_plates(chieftain_t *self, int berserker)
{
    while (1)
    {
        int achou_lugar = 0;
        int cadeira = -1;
        pthread_mutex_lock(&self->mesa_mutex);

        for (int i = 0; i < config.table_size; i++)
        {
            if (self->mesa[i] == 0)
            {
                int seguro = 1;
                if (!berserker)
                {
                    if (i > 0 && self->mesa[i - 1] == 2)
                    {
                        seguro = 0;
                    }
                    if (i < config.table_size - 1 && self->mesa[i + 1] == 2)
                    {
                        seguro = 0;
                    }
                }
                else
                {
                    if (i > 0 && self->mesa[i - 1] == 1)
                    {
                        seguro = 0;
                    }
                    if (i < config.table_size - 1 && self->mesa[i + 1] == 1)
                    {
                        seguro = 0;
                    }
                }
                int prato_esq = (i - 1 + config.table_size) % config.table_size;
                int prato_meio = i;
                int prato_dir = (i + 1) % config.table_size;

                int pratos_livres = 0;
                if (self->pratos[prato_esq] == 0)
                    pratos_livres++;
                if (self->pratos[prato_meio] == 0)
                    pratos_livres++;
                if (self->pratos[prato_dir] == 0)
                    pratos_livres++;
                if (seguro && pratos_livres >= 2)
                {
                    self->mesa[i] = berserker ? 2 : 1; // 1 = normal, 2 = berserker

                    cadeira = i;
                    achou_lugar = 1;

                    int p1 = -1, p2 = -1;
                    if (self->pratos[prato_esq] == 0)
                    {
                        p1 = prato_esq;
                    }
                    if (self->pratos[prato_meio] == 0)
                    {
                        if (p1 == -1)
                        {
                            p1 = prato_meio;
                        }
                        else if (p2 == -1)
                        {
                            p2 = prato_meio;
                        }
                    }
                    if (self->pratos[prato_dir] == 0)
                    {
                        if (p1 == -1)
                        {
                            p1 = prato_dir;
                        }
                        else if (p2 == -1)
                        {
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
        if (achou_lugar)
        {
            pthread_mutex_unlock(&self->mesa_mutex);
            return cadeira;
        }
        else
        {
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

    if (prato1 != -1)
    {
        self->pratos[prato1] = 0; // Marca o prato como livre
    }
    if (prato2 != -1)
    {
        self->pratos[prato2] = 0; // Marca o prato como livre
    }

    self->prato1_da_cadeira[pos] = -1;
    self->prato2_da_cadeira[pos] = -1;

    while (self->vikings_esperando > 0)
    {
        sem_post(&self->fila_espera);
        self->vikings_esperando--;
    }

    self->vikings_que_ja_comeram++;

    if (self->vikings_que_ja_comeram == config.horde_size)
    {
        sem_post(&self->preces_barrier);
    }

    pthread_mutex_unlock(&self->mesa_mutex);
}

god_t chieftain_get_god(chieftain_t *self)
{
    /*Wait para esperar o ultimo viking (da função chieftain_release_seat_plates) terminar de comer para rezar*/
    sem_wait(&self->preces_barrier);
    /*Quando acontece post lá na função chieftain_release_seat_plates a barreira abre para rezar e SOMENTE UM viking passa para rezar*/

    /*Aqui abre a barreira para outro viking poder rezar*/
    sem_post(&self->preces_barrier);

    pthread_mutex_lock(&self->autorizacao_de_rezar_mutex);
    int has_god = 0;
    god_t god;

    while (!has_god)
    {
        god = rand() % NUMBER_OF_GODS;
        if (!valhalla_is_super(god))
        {
            int rival = valhalla_get_rival(god);
            int maximo_permitido = (self->preces_autorizadas[rival] == 0) ? 1 : (int)ceil(self->preces_autorizadas[rival] * (1.0 + RIVAL_TOLERANCE_RATE));
            /* self->preces_autorizadas[god] + 1 para simular a prece atual*/
            if (self->preces_autorizadas[god] + 1 <= maximo_permitido)
            {
                self->preces_autorizadas[god]++;
                has_god = 1;
            }
        }
        else
        {
            int conta_preces = 0;
            for (int i = 0; i < NUMBER_OF_GODS - 2; i++)
            {
                conta_preces += self->preces_autorizadas[i];
            }
            int maximo_permitido = (int)ceil(conta_preces * (1.0 + SUPER_GOD_TOLERANCE_RATE));
            if (self->preces_autorizadas[god] + 1 <= maximo_permitido)
            {
                self->preces_autorizadas[god]++;
                has_god = 1;
            }
        }
    }
    pthread_mutex_unlock(&self->autorizacao_de_rezar_mutex);
    return god;
}

void chieftain_finalize(chieftain_t *self)
{
    free(self->mesa);
    free(self->pratos);
    free(self->prato1_da_cadeira);
    free(self->prato2_da_cadeira);
    pthread_mutex_destroy(&self->mesa_mutex);
    pthread_mutex_destroy(&self->autorizacao_de_rezar_mutex);
    sem_destroy(&self->fila_espera);
    sem_destroy(&self->preces_barrier);

    plog("[chieftain] Finalized\n");
}
