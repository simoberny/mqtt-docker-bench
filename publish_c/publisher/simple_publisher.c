
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
    Generate temperature and humidity feedback with random value
*/
char * generate_string(){
    int temp = (rand() % (25 - 0 + 1)) + 0;        
    int hum = (rand() % (100 - 20 + 1)) + 20;

    /* print a message */
    char * application_message = malloc(100);
    snprintf(application_message, 100, "%d C° - Humidity: %d %%", temp, hum);

    return application_message;
}

/**
    Random string with payload len defined
*/
char * gen_random(const int len) {
    char * application_message = malloc(len);

    char alphanum[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";

    for (int i = 0; i < len; ++i) {
        application_message[i] = alphanum[rand() % (sizeof(alphanum) - 1)];
    }

    application_message[len] = 0;

    return application_message;
}

int main(int argc, const char *argv[]) 
{
    srand(time(0));

    const char* addr;
    const char* port;
    const char* topic;
    int method = 0;
    int sink = 0;
    int sensor_per_sink = 0;
    int message_length = 0;
    int qos_level = MQTT_PUBLISH_QOS_0;

    /*
        Arg parsing
    */
    if (argc > 1) {
        // Propagation method
        method = atoi(argv[1]);
    }

    if (argc > 2) {
        // Sink node or not
        sink = atoi(argv[2]);
    }

    if (argc > 3) {
        // Number of sensors behind sink
        sensor_per_sink = atoi(argv[3]);
    }

    if (argc > 4) {
        // Custom payload size
        message_length = atoi(argv[4]);
    }

    addr = "192.168.178.47"; //Address of the broker
    port = "1883";
    topic = "test";

    char hostname[1024];
    hostname[1023] = '\0';
    gethostname(hostname, 1023);

    int sockfd = open_nb_socket(addr, port);

    if (sockfd == -1) {
        perror("Failed to open socket: ");
        // exit_example(EXIT_FAILURE, sockfd, NULL);
    }

    /* setup a client */
    struct mqtt_client client;
    uint8_t sendbuf[16384]; /* sendbuf should be large enough to hold multiple whole mqtt messages */
    uint8_t recvbuf[1024]; /* recvbuf should be large enough any whole mqtt message expected to be received */

    mqtt_init(&client, sockfd, sendbuf, sizeof(sendbuf), recvbuf, sizeof(recvbuf), publish_callback);
    const char* client_id = hostname;

    /* Ensure we have a clean session */
    uint8_t connect_flags = MQTT_CONNECT_CLEAN_SESSION;
    /* Send connection request to the broker. */

    mqtt_connect(&client, client_id, NULL, NULL, 0, NULL, NULL, connect_flags, 120);

    /* check that we don't have any errors */
    if (client.error != MQTT_OK) {
        fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
        // exit_example(EXIT_FAILURE, sockfd, NULL);
    }

    // Publishing infinite loop
    while(1){           
        if(sink){ //Sink loop
            // Sends all the messsage without waiting
            for(int i = 0; i < sensor_per_sink; i++){
                char *application_message = generate_string();
                mqtt_publish(&client, topic, application_message, strlen(application_message) + 1, qos_level);
            }
            printf("Sinks published %d message\n", sensor_per_sink);
        }else if(message_length > 0){
            //Default message
            char *application_message = gen_random(message_length);

            mqtt_publish(&client, topic, application_message, strlen(application_message) + 1, qos_level);
            printf("%s published : \"%s\"\n", hostname, application_message);
        }else{
            //Custom payload message
            char *application_message = generate_string();

            mqtt_publish(&client, topic, application_message, strlen(application_message) + 1, qos_level);
            printf("%s published : \"%s\"\n", hostname, application_message);
        }

        /* check for errors */
        if (client.error != MQTT_OK) {
            fprintf(stderr, "error: %s\n", mqtt_error_str(client.error));
            // exit_example(EXIT_FAILURE, sockfd, &client_daemon);
        }

        mqtt_sync(&client);

        if(method == 0){
            // Waiting defined time
            sleep(1);
        }else{
            // Waiting realistic time
            int wait = (rand() % (5 - 0 + 1)) + 0;
            sleep(wait);
        }   
    }

    /* exit */ 
    // exit_example(EXIT_SUCCESS, sockfd, &client_daemon);
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