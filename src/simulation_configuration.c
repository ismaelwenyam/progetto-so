#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "simerr.h"
#include "simulation_configuration.h"
#include "semapi.h"

ConfigurationAdt get_config()
{
    ConfigurationAdt configuration;

    FILE *configurationFile = fopen("simulation_configuration.conf", "r");
    if (configurationFile == NULL)
    {
        printf("error in opening configuration file\n");
        err_exit(strerror(errno));
    }

    char line[256];
    int lineNum = 0;
    while (fgets(line, 256, configurationFile) != NULL)
    {
        char key[256], value[256];
        lineNum++;
        if (line[0] == '#')
            continue;
        if (sscanf(line, "%s %s", key, value) != 2)
        {
            fprintf(stderr, "syntax error in config file %s at line %d\n", "config_sim.conf", lineNum);
            err_exit("error in configuration file");
        }
        if (strcmp(key, "nof_worker_seats") == 0)
        {
            configuration.nofWorkerSeats = atoi(value);
        }
        else if (strcmp(key, "nof_workers") == 0)
        {
            configuration.nofWorkers = atoi(value);
        }
        else if (strcmp(key, "nof_users") == 0)
        {
            configuration.nofUsers = atoi(value);
        }
        else if (strcmp(key, "sim_duration") == 0)
        {
            configuration.simDuration = atoi(value);
        }
        else if (strcmp(key, "n_nano_secs") == 0)
        {
            configuration.nNanoSecs = atoi(value);
        }
        else if (strcmp(key, "nof_pause") == 0)
        {
            configuration.nofPause = atoi(value);
        }
        else if (strcmp(key, "p_serv_min") == 0)
        {
            configuration.pServMin = atoi(value);
        }
        else if (strcmp(key, "p_serv_max") == 0)
        {
            configuration.pServMax = atoi(value);
        }
        else if (strcmp(key, "explode_threshold") == 0)
        {
            configuration.explodeThreshold = atoi(value);
        }
        else if (strcmp(key, "n_requests") == 0)
        {
            configuration.nRequests = atoi(value);
        }
        else
        {
            configuration.nNewUsers = atoi(value);
        }
    }

    fclose(configurationFile);
    return configuration;
}

int update_timeout(int semId, int d)
{
    if (reserve_sem(semId, 0) == -1)
    {
        printf("error reserving config sem\n");
        err_exit(strerror(errno));
    }

    FILE *configFile = fopen("config_timout.conf", "w");
    if (configFile == NULL)
    {
        printf("error in opening config_timout file\n");
        return -1;
    }
    fprintf(configFile, "%s %d\n", "timeout", d);

    fclose(configFile);
    if (release_sem(semId, 0) == -1)
    {
        printf("error releasing config sem\n");
        err_exit(strerror(errno));
    }
    return 0;
}

int reset_explode(int semId)
{
    if (reserve_sem(semId, 0) == -1)
    {
        printf("error reserving config sem\n");
        err_exit(strerror(errno));
    }

    FILE *configFile = fopen("config_explode.conf", "w");
    if (configFile == NULL)
    {
        printf("error in opening config_explode file\n");
        return -1;
    }
    fprintf(configFile, "%s %d\n", "explode", 0);

    fclose(configFile);

    if (release_sem(semId, 0) == -1)
    {
        printf("error releasing config sem\n");
        err_exit(strerror(errno));
    }
    return 0;
}

int update_explode(int semId, int count)
{
    if (reserve_sem(semId, 0) == -1)
    {
        printf("error reserving config sem\n");
        err_exit(strerror(errno));
    }

    FILE *configFile = fopen("config_explode.conf", "r+");
    if (configFile == NULL)
    {
        printf("error in opening config_explode file\n");
        return -1;
    }

    int explode = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int lineNum = 0;

    while ((read = getline(&line, &len, configFile)) != -1)
    {
        lineNum++;
        char key[256], value[256];
        if (line[0] == '#')
            continue; // Salta i commenti
        if (sscanf(line, "%s %s", key, value) != 2)
        {
            fprintf(stderr, "syntax error in config file %s at line %d\n", "config_explode.conf", lineNum);
            free(line);
            fclose(configFile);
            return -1;
        }
        explode = atoi(value);
    }

    free(line);

    rewind(configFile);

    fprintf(configFile, "explode %d\n", explode + count);

    fclose(configFile);
    if (release_sem(semId, 0) == -1)
    {
        printf("error releasing config sem\n");
        err_exit(strerror(errno));
    }
    return 0;
}

int get_timeout(int semId)
{

    if (reserve_sem(semId, 0) == -1)
    {
        fprintf(stderr, "Error reserving config sem: %s\n", strerror(errno));
        err_exit("Failed to reserve semaphore");
    }

    FILE *configFile = fopen("config_timout.conf", "r");
    if (configFile == NULL)
    {
        fprintf(stderr, "Error opening config_timeout file: %s\n", strerror(errno));
        release_sem(semId, 0);
    }

    int timeout = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int lineNum = 0;

    while ((read = getline(&line, &len, configFile)) != -1)
    {
        lineNum++;

        if (line[0] == '#')
            continue;

        char key[256], value[256];
        if (sscanf(line, "%s %s", key, value) != 2)
        {
            fprintf(stderr, "Syntax error in config file %s at line %d\n", "config_timeout.conf", lineNum);
            free(line);
            fclose(configFile);
            release_sem(semId, 0);
            err_exit("Invalid configuration file");
        }

        timeout = atoi(value);
    }

    free(line);

    if (fclose(configFile) != 0)
    {
        fprintf(stderr, "Error closing config file: %s\n", strerror(errno));
        release_sem(semId, 0);
        err_exit("Failed to close configuration file");
    }

    if (release_sem(semId, 0) == -1)
    {
        fprintf(stderr, "Error releasing config sem: %s\n", strerror(errno));
        err_exit("Failed to release semaphore");
    }
    return timeout;
}

int get_explode(int semId)
{
    if (reserve_sem(semId, 0) == -1)
    {
        fprintf(stderr, "Error reserving config sem: %s\n", strerror(errno));
        err_exit("Failed to reserve semaphore");
    }

    FILE *configFile = fopen("config_explode.conf", "r");
    if (configFile == NULL)
    {
        fprintf(stderr, "Error opening config_timeout file: %s\n", strerror(errno));
        release_sem(semId, 0);
        return -1;
    }

    int explode = 0;
    char *line = NULL;
    size_t len = 0;
    ssize_t read;
    int lineNum = 0;

    while ((read = getline(&line, &len, configFile)) != -1)
    {
        lineNum++;

        if (line[0] == '#')
            continue;

        char key[256], value[256];
        if (sscanf(line, "%s %s", key, value) != 2)
        {
            fprintf(stderr, "Syntax error in config file %s at line %d\n", "config_explode.conf", lineNum);
            free(line);
            fclose(configFile);
            release_sem(semId, 0);
            err_exit("Invalid configuration file");
        }

        explode = atoi(value);
    }

    free(line);

    if (fclose(configFile) != 0)
    {
        fprintf(stderr, "Error closing config file: %s\n", strerror(errno));
        release_sem(semId, 0);
        err_exit("Failed to close configuration file");
    }

    if (release_sem(semId, 0) == -1)
    {
        fprintf(stderr, "Error releasing config sem: %s\n", strerror(errno));
        err_exit("Failed to release semaphore");
    }
    return explode;
}
