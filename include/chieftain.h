#ifndef __CHIEFTAIN_H__
#define __CHIEFTAIN_H__
    #include <semaphore.h>
    #include "config.h"
    #include "valhalla.h"

    /*============================================================================*
     * DESCRIÇÃO: O chieftain é o chefe da horda de vikings. É através dele que   *
     * os vikings pedem cadeiras e pratos (e os devolvem) e também solicitam      *
     * deuses para realizar as preces.                                            *
    *============================================================================*/

    /**
     * @brief Define os atributos do chieftain.
     */
    typedef struct chieftain
    {
        valhalla_t *valhalla;   /* Referência para valhalla.  */

        int *mesa; /* Array que representa a mesa e as cadeiras ocupadas. (0 = livre, 1 = normal, 2 = berserker ) */
        int *pratos; /* Array que representa os pratos ocupados. (0 = livre, 1 = ocupado)*/

        int *prato1_da_cadeira; /* Array que mapeia cada cadeira para o índice do primeiro prato associado a ela. */ 
        int *prato2_da_cadeira; /* Array que mapeia cada cadeira para o índice do segundo prato associado a ela. */

        /*Importante para o caso que o viking 1 solicitar o deus, autorizar e antes de realizar a prece (prayers[god]++),
        o viking 2 solicitar o mesmo deus, mas podendo não ser autorizado */
        int preces_autorizadas[NUMBER_OF_GODS]; /* Controla o número de preces autorizadas para cada deus. */

        pthread_mutex_t mesa_mutex; /* Mutex para proteger o acesso à mesa e aos pratos. */
        pthread_mutex_t autorizacao_de_rezar_mutex; /* Mutex para proteger a autorização de rezar. */

        sem_t fila_espera; /* Semáforo para controlar o número de vikings na mesa. */
        int vikings_esperando; /* Contador de vikings esperando para sentar. */

        int vikings_que_ja_comeram; /* Contador de vikings que já comeram. */
        sem_t preces_barrier; /* Semáforo para controlar a barreira de preces. */
    } chieftain_t;

    /*============================================================================*
     * Funções utilizadas em arquivos que incluem esse .h                         *
     *============================================================================*/

    /**
     * @brief Inicializa o chieftain (o chefe da horda). Ele controlará
     * uma horda com inicialmente config.horde_size guerreiros e uma mesa com config.table_size
     * cadeiras.
     * 
     * @param self O chieftain.
     * @param valhalla Valhalla.
     */
    extern void chieftain_init(chieftain_t *self, valhalla_t *valhalla);
    
    /**
     * @brief Finaliza o chieftain (o chefe da horda).
     * 
     * @param self O chieftain.
    */
    extern void chieftain_finalize(chieftain_t *self);

    /**
     * @brief Adquire uma cadeira e dois pratos de comida. 
     * 
     * @param self O chieftain.
     * @param berserker Se igual a 1 indica que o viking que chamou este método é um
     * berserker. Por razões de segurança, guerreiros normais e berserkers jamais podem
     * ser atribuídos pelo chieftain a cadeiras adjacentes.
     *
     * @returns Índice da cadeira a ser ocupada pelo guerreiro, como um número no
     * intervalo [0, config.table_size)
     */
    extern int chieftain_acquire_seat_plates(chieftain_t *self, int berserker);

    /**
     * @brief Libera uma cadeira e dois pratos previamente adquiridos via
     * chieftain_acquire_seat_plates().
     *
     * @param self O chieftain.
     * @param pos Índice da cadeira ocupada pelo guerreiro, como um número no
     * intervalo [0, config.table_size)
     */
    extern void chieftain_release_seat_plates(chieftain_t *self, int pos);

    /**
     * @brief Indica ao guerreiro para qual deus viking ele deve fazer suas preces. O deus
     * deve ser escolhido aleatóriamente, mas de forma a respeitar as regras dispostas nas
     * escrituras (ver enunciado).
     *
     * @param self O chieftain.
     * 
     * @returns O Deus atribuído ao guerreiro.
     */
    extern god_t chieftain_get_god(chieftain_t *self);

    /*============================================================================*
    * ATENCÃO: Insira aqui funções que você quiser adicionar a interface para    *
    * serem usadas em arquivos que incluem esse header.                          *
    *============================================================================*/

    /**
     * @brief Verifica se uma cadeira é segura para um viking.
     *
     * @param self O chieftain.
     * @param pos Índice da cadeira.
     * @param berserker Se igual a 1 indica que o viking é um berserker.
     *
     * @returns 1 se a cadeira é segura, 0 caso contrário.
     */
    extern int is_seat_safe(chieftain_t *self, int pos, int berserker);

    /**
     * @brief Tenta adquirir dois pratos para a cadeira especificada.
     * 
     * @param self O chieftain.
     * @param pos Índice da cadeira.
     * @param p1 Ponteiro para o primeiro prato.
     * @param p2 Ponteiro para o segundo prato.
     *
     * @returns 1 se os pratos foram adquiridos, 0 caso contrário.
     */
    extern int try_take_plates(chieftain_t *self, int pos, int *p1, int *p2);

    extern int can_pray_god(chieftain_t *self, god_t god);

#endif /*__CHIEFTAIN_H__*/