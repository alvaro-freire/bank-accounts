#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "options.h"

#define MAX_AMOUNT 20

struct bank {
    int num_accounts;        // number of accounts
    int *accounts;           // balance array
    bool tranfers_finished;
    pthread_mutex_t *mutex;  // mutex array
};

struct args {
    int thread_num;  // application defined thread #
    int delay;       // delay between operations
    int iterations;  // number of operations
    int net_total;   // total amount deposited by this thread
    struct bank *bank;        // pointer to the bank (shared with other threads)
};

struct thread_info {
    pthread_t id;    // id returned by pthread_create()
    struct args *args;  // pointer to the arguments
};

// Threads run on this function
void *deposit(void *ptr) {
    struct args *args = ptr;
    int amount, account, balance;

    while (args->iterations--) {
        amount = rand() % MAX_AMOUNT;
        account = rand() % args->bank->num_accounts;

        printf("Thread %d depositing %d on account %d\n",
               args->thread_num, amount, account);

        /* Inicio sección crítica */

        pthread_mutex_lock(&args->bank->mutex[account]);

        balance = args->bank->accounts[account];
        if (args->delay) usleep(args->delay); // Force a context switch

        balance += amount;
        if (args->delay) usleep(args->delay);

        args->bank->accounts[account] = balance;
        if (args->delay) usleep(args->delay);

        args->net_total += amount;

        pthread_mutex_unlock(&args->bank->mutex[account]);

        /* Fin sección crítica */
    }
    return NULL;
}

void *transfer(void *ptr) {
    struct args *args = ptr;
    int amount, account1, account2;
    int tmp = args->iterations;


    while (tmp--) {
        do {
            account1 = rand() % args->bank->num_accounts;
        } while (args->bank->accounts[account1] == 0);

        do {
            account2 = rand() % args->bank->num_accounts;
        } while (account1 == account2);

        while (1) {
            /* Inicio sección crítica */

            /* prevención interbloqueo entre account1 y account2: */
            pthread_mutex_lock(&args->bank->mutex[account1]);
            if (pthread_mutex_trylock(&args->bank->mutex[account2])) {
                pthread_mutex_unlock(&args->bank->mutex[account1]);
                usleep(1);
                continue;
            }

            // if bank account1 is empty, transfer is not possible
            if (args->bank->accounts[account1] == 0 || (amount = rand() % args->bank->accounts[account1]) == 0) {
                tmp++;

                pthread_mutex_unlock(&args->bank->mutex[account1]);
                pthread_mutex_unlock(&args->bank->mutex[account2]);
                break;
            }

            args->bank->accounts[account1] -= amount;
            if (args->delay) usleep(args->delay); // Force a context switch

            args->bank->accounts[account2] += amount;
            if (args->delay) usleep(args->delay);


            pthread_mutex_unlock(&args->bank->mutex[account1]);
            pthread_mutex_unlock(&args->bank->mutex[account2]);

            printf("Thread %d transferring %d from account %d to account %d\n",
                   args->thread_num, amount, account1, account2);

            break;
            /* Fin sección crítica */
        }

    }
    return NULL;
}

// start opt.num_threads threads running on deposit.
struct thread_info *start_threads(struct options opt, struct bank *bank, void *(operation)(void *)) {
    int i;
    struct thread_info *threads;

    printf("creating %d threads\n", opt.num_threads);
    threads = malloc(sizeof(struct thread_info) * opt.num_threads);

    if (threads == NULL) {
        printf("Not enough memory\n");
        exit(1);
    }

    // Create num_thread threads running swap()
    for (i = 0; i < opt.num_threads; i++) {
        threads[i].args = malloc(sizeof(struct args));

        threads[i].args->thread_num = i;
        threads[i].args->net_total = 0;
        threads[i].args->bank = bank;
        threads[i].args->delay = opt.delay;
        threads[i].args->iterations = opt.iterations;

        if (0 != pthread_create(&threads[i].id, NULL, operation, threads[i].args)) {
            printf("Could not create thread #%d", i);
            exit(1);
        }
    }

    return threads;
}

// Print the final balances of accounts and threads
void print_net_deposits(struct thread_info *thrs, int num_threads) {
    int total_deposits = 0;
    printf("\nNet deposits by thread\n");

    for (int i = 0; i < num_threads; i++) {
        printf("%d: %d\n", i, thrs[i].args->net_total);
        total_deposits += thrs[i].args->net_total;
    }
    printf("Total: %d\n", total_deposits);
}

void print_balance(struct bank *bank) {
    int bank_total = 0;

    printf("\nAccount balance\n");
    for (int i = 0; i < bank->num_accounts; i++) {
        printf("%d: %d\n", i, bank->accounts[i]);
        bank_total += bank->accounts[i];
    }
    printf("Total: %d\n\n", bank_total);
}

void wait(int num_threads, struct bank *bank, struct thread_info *threads, bool tranfers_finished) {
    // Wait for the threads to finish
    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i].id, NULL);
    }

    if (tranfers_finished) {
        // Raise flag (all threads have finished)
        bank->tranfers_finished = true;
    } else {
        // wait call after deposits:
        print_net_deposits(threads, num_threads);
    }

    print_balance(bank);

    for (int i = 0; i < num_threads; i++) {
        free(threads[i].args);
    }

    free(threads);
}

void destroy(struct bank *bank) {
    int i;

    for (i = 0; i > bank->num_accounts; i++) {
        pthread_mutex_destroy(&bank->mutex[i]);
    }

    free(bank->mutex);
    free(bank->accounts);
}

// allocate memory, and set all accounts to 0
void init_accounts(struct bank *bank, int num_accounts) {
    bank->num_accounts = num_accounts;
    bank->accounts = malloc(bank->num_accounts * sizeof(int));
    bank->mutex = malloc(bank->num_accounts * sizeof(pthread_mutex_t));

    for (int i = 0; i < bank->num_accounts; i++) {
        bank->accounts[i] = 0;
        pthread_mutex_init(&bank->mutex[i], NULL);
    }
}

int main(int argc, char **argv) {
    struct options opt;
    struct bank bank;
    struct thread_info *thrs_d; // threads for deposits
    struct thread_info *thrs_t; // threads for transfers

    srand(time(NULL));

    // Default values for the options
    opt.num_threads = 5;
    opt.num_accounts = 10;
    opt.iterations = 100;
    opt.delay = 10;

    read_options(argc, argv, &opt);

    init_accounts(&bank, opt.num_accounts);

    printf("**** DEPOSITS ****\n");
    thrs_d = start_threads(opt, &bank, deposit);
    wait(opt.num_threads, &bank, thrs_d, false);

    printf("**** TRANSFERS ****\n");
    thrs_t = start_threads(opt, &bank, transfer);   // threads for transfers
    wait(opt.num_threads, &bank, thrs_t, true);

    destroy(&bank);

    return 0;
}
