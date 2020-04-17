
/**
 * @file
 * A simple program to that publishes the current time whenever ENTER is pressed. 
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h> 
#include <mqtt.h>
#include "templates/posix_sockets.h"


/**
 * @brief The function that would be called whenever a PUBLISH is received.
 * 
 * @note This function is not used in this example. 
 */
void publish_callback(void** unused, struct mqtt_response_publish *published);

/**
 * @brief The client's refresher. This function triggers back-end routines to 
 *        handle ingress/egress traffic to the broker.
 * 
 * @note All this function needs to do is call \ref __mqtt_recv and 
 *       \ref __mqtt_send every so often. I've picked 100 ms meaning that 
 *       client ingress/egress traffic will be handled every 100 ms.
 */
void* client_refresher(void* client);

/**
 * @brief Safelty closes the \p sockfd and cancels the \p client_daemon before \c exit. 
 */
void exit_example(int status, int sockfd, pthread_t *client_daemon);

/**
 * A simple program to that publishes the current time whenever ENTER is pressed. 
 */
int main(int argc, const char *argv[]) 
{
    srand(time(0));

    const char* addr;
    const char* port;
    const char* topic;
    int method = 0;
    int sink = 0;
    int sensor_per_sink = 0;

    if (argc > 1) {
        method = atoi(argv[1]);
    }

    if (argc > 2) {
        sink = atoi(argv[2]);
    }

    if (argc > 3) {
        sensor_per_sink = atoi(argv[3]);
    }

    addr = "192.168.178.47";
    port = "1883";
    topic = "test";

    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);

    int sockfd = open_nb_socket(addr, port);

    if (sockfd == -1) {
        perror("Failed to open socket: ");
        exit_example(EXIT_FAILURE, sockfd, NULL);
    }

    /* setup a client */
    struct mqtt_client client;
    uint8_t sendbuf[16384]; /* sendbuf should be large enough to hold multiple whole mqtt messages */
    uint8_t recvbuf[1024]; /* recvbuf should be large enough any whole mqtt message expected to be received */

    mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), publish_callback);
    
    /* Create an anonymous session */
    const char* client_id = hostname;

    /* Ensure we have a clean session */
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    /* Send connection request to the broker. */

    mqtt_connect(&client, client_id, NULL, NULL, 0, NULL, NULL, connect_flags, 120);

    /* check that we don't have any errors */
    if (client.error != MQTT_OK) {
        fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
        exit_example(EXIT_FAILURE, sockfd, NULL);
    }

    /* start a thread to refresh the client (handle egress and ingree client traffic) */
    pthread_t client_daemon;
    if(pthread_create(&client_daemon, NULL, client_refresher, &client)) {
        fprintf(stderr, "Failed to start client daemon.\n");
        exit_example(EXIT_FAILURE, sockfd, NULL);
    }

    // Publishing infinite loop
    while(1){   
        int temp = (rand() % (25 - 0 + 1)) + 0;        
        int hum = (rand() % (100 - 20 + 1)) + 20;

        /* print a message */
        char application_message[256];
        snprintf(application_message, sizeof(application_message), "%d C° - Humidity: %d %%", temp, hum);
        
        if(sink){
            for(int i = 0; i < sensor_per_sink; i++){
                mqtt_publish(&client, topic, application_message, strlen(application_message) + 1, MQTT_PUBLISH_QOS_0);
            }
            printf("Sinks published %d message\n", sensor_per_sink);
        }else{
            mqtt_publish(&client, topic, application_message, strlen(application_message) + 1, MQTT_PUBLISH_QOS_0);
            printf("%s published : \"%s\"\n", hostname, application_message);
        }

        /* check for errors */
        if (client.error != MQTT_OK) {
            fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
            exit_example(EXIT_FAILURE, sockfd, &client_daemon);
        }

        if(method == 0){
            sleep(1);
        }else{
            int wait = (rand() % (5 - 0 + 1)) + 0;
            sleep(wait);
        }
        
    }

    /* exit */ 
    exit_example(EXIT_SUCCESS, sockfd, &client_daemon);
}

void exit_example(int status, int sockfd, pthread_t *client_daemon)
{
    if (sockfd != -1) close(sockfd);
    if (client_daemon != NULL) pthread_cancel(*client_daemon);
    exit(status);
}


void publish_callback(void** unused, struct mqtt_response_publish *published) 
{
    /* not used in this example */
}

void* client_refresher(void* client)
{
    while(1) 
    {
        mqtt_sync((struct mqtt_client*) client);
        usleep(100000U);
    }
    return NULL;
}