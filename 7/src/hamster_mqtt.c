#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <inttypes.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <assert.h>
#include "../include/MQTTClient.h"

#include <stdio.h>
#include <stdlib.h> // For exit()
/**
 * @brief Debug support macros
 *
 * Output messages if @c verbose is higher than a given level.
 *
 * NOTE: Usage @c debug((<printf-params>)); i.e. use @e two brackets
 *       to enclose the printf parameter list!
 */
#ifdef EBUG
#define debug(x)        \
	do                  \
	{                   \
		printf x;       \
		fflush(stdout); \
	} while (0)
#else
#define debug(x)
#endif

#define HMSTR_MAX_NAME 31
static char clientid[32];

#define RUNNING 0
#define SLEEPING 1
#define EATING 2
#define MATING 3

// kai.beckmann@hs-rm.de

/*
 * Code stolen and adapted from 5/sunrpc/hamster_cli.c:
 */
static void cli_commands()
{
	printf("\nCommands:\n");
	printf("        A                           - Go to room A\n");
	printf("        B                           - Go to room B\n");
	printf("        C                           - Go to room C\n");
	printf("        D                           - Go to room D\n");
	printf("        r                           - change state to RUNNING\n");
	printf("        s                           - change state to SLEEPING\n");
	printf("        e                           - change state to EATING\n");
	printf("        m                           - change state to MATING\n");
	printf("        q                           - terminate hamster\n");
	printf("> ");
	fflush(stdout);
}

static void flushinput(void)
{
	while (getchar() != '\n')
		;
}

void rtfm(char **argv)
{
	printf("Usage: %s hamster_client_id {<Options>}\n", argv[0]);
	printf("Function: Hamster instrumentation device software\n");
	printf("Options:\n");
	printf("     -p <port>                      - port of the mqtt server (default (no tls): 1883\n");
	printf("     -s <IP address>                - IP address to run the server on (default: 127.0.0.1\n");
	printf("     -v                             - Connect with Certificate based SSL/TLS Support to the MQTT server \n");
	printf("     -V                             - Connect with Certificate based client authentification to the MQTT server \n");
	printf("     -o <owner name>                - Hamster owner's name (default: $USER)\n");
	printf("     -n <hamster name>              - Hamster's name (default: myhamster)\n");
	printf("     -i <hamster ID>                - Hamster's ID (overrides owner & hamster names)\n");
	printf("     -h                             - This help \n");
}

#define INITIAL_VALUE 5381
#define M 33

/**
 * @brief generates a hash for 2 strings
 * 
 * @param s1 
 * @param s2 
 * @return int 
 */
static int make_hash(const char *s1, const char *s2)
{
	unsigned int hash;
	int i;
	char key[2 * (HMSTR_MAX_NAME + 1)];

	/* copy both strings into a single buffer */
	memset(key, 0, sizeof(key));
	strcpy(&key[0], s1);
	strcpy(&key[HMSTR_MAX_NAME + 1], s2);

	/* compute a hash value over the buffer's contents */
	hash = INITIAL_VALUE;
	for (i = 0; i < 2 * (HMSTR_MAX_NAME + 1); ++i)
		hash = M * hash + key[i];

	/* make sure always get a >= 0 number */
	return hash >> 1U;
}

/**
 * @brief Calculates the number of wheels for a time period
 * 
 * @param startTime 
 * @param endTime 
 * @return int 
 */
int calculateWheels(time_t startTime, time_t endTime)
{
	double diff = difftime(endTime, startTime);
	return (int)(diff / 60 * 25);
}

volatile MQTTClient_deliveryToken deliveredtoken;
/**
 * @brief prints to terminal when a message is delivered
 * 
 * @param context 
 * @param dt 
 */
void delivered(void *context, MQTTClient_deliveryToken dt)
{
	printf("Message with token value %d delivery confirmed\n", dt);
	deliveredtoken = dt;
}

/**
 * @brief prints to terminal when a message has arrived
 * 
 * @param context 
 * @param topicName 
 * @param topicLen 
 * @param message 
 * @return int 
 */
int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
	int i;
	char *payloadptr;
	printf("Message arrived\n");
	printf("     topic: %s\n", topicName);

	if (strstr(topicName, "/fondle"))
	{
		printf("Der Hamster wurde ");
		payloadptr = message->payload;

		for (i = 0; i < message->payloadlen; i++)
		{
			putchar(*payloadptr++);
		}
		printf(" geknuddelt\n");
	}
	else if (strstr(topicName, "/punish"))
	{
		printf("Der Hamster wurde ");
		payloadptr = message->payload;

		for (i = 0; i < message->payloadlen; i++)
		{
			putchar(*payloadptr++);
		}
		printf(" bestraft\n");
	}
	else if (strstr(topicName, "/room/"))
	{

		payloadptr = message->payload;
		for (i = 0; i < message->payloadlen; i++)
		{
			putchar(*payloadptr++);
		}
	}
	else
	{
		payloadptr = message->payload;
		for (i = 0; i < message->payloadlen; i++)
		{
			putchar(*payloadptr++);
		}
	}
	MQTTClient_freeMessage(&message);
	MQTTClient_free(topicName);
	return 1;
}

/**
 * @brief prints to terminal when the connection is terminated
 * 
 * @param context 
 * @param cause 
 */
void connlost(void *context, char *cause)
{
	printf("\nConnection lost\n");
	printf("     cause: %s\n", cause);
}

/*
 * Declare these static global to suppress compiler warnings
 * about variables being "set but not used". Ultimately, your
 * program *should* use these variables so you should be able
 * move these into main() without warnings.
 */
static int doAuthenticate = 0;
static int doEncrypt = 0;
static unsigned int port = 1883;
static char *ipaddr = "127.0.0.1";
// static char *ipaddr = "hamsteriot.vs.cs.hs-rm.de";

#define TIMEOUT 1000

/**
 * @brief publishes a message
 * 
 * @param client 
 * @param topic 
 * @param payload 
 * @param qos 
 * @return int 
 */
int publish(MQTTClient *client, char *topic, char *payload, int qos)
{
	MQTTClient_deliveryToken token;

	MQTTClient_message pubmsg = MQTTClient_message_initializer;
	pubmsg.payload = payload;
	pubmsg.payloadlen = strlen(payload);
	pubmsg.qos = qos;
	pubmsg.retained = 0;

	MQTTClient_publishMessage(*client, topic, &pubmsg, &token);
	int rc = MQTTClient_waitForCompletion(*client, token, TIMEOUT);
	return rc;
}

/**
 * @brief publishes a Message to the topic position
 * 
 * @param client 
 * @param hamsterID 
 * @param payload 
 * @return int 
 */
int publishRoom(MQTTClient *client, int hamsterID, char *payload)
{
	char topic[40];
	memset(topic, 0, 40);
	sprintf(topic, "/pension/hamster/%d/position", hamsterID);

	return publish(client, topic, payload, 1);
}

/**
 * @brief publishes a message to the topic state
 * 
 * @param client 
 * @param hamsterID 
 * @param payload 
 * @return int 
 */
int publishState(MQTTClient *client, int hamsterID, char *payload)
{
	char topic[40];
	memset(topic, 0, 40);
	sprintf(topic, "/pension/hamster/%d/state", hamsterID);

	return publish(client, topic, payload, 1);
}

/**
 * @brief publishes a message to the topic wheels
 * 
 * @param client 
 * @param hamsterID 
 * @param wheels 
 * @return int 
 */
int publishWheels(MQTTClient *client, int hamsterID, int wheels)
{
	char topic[40];
	memset(topic, 0, 40);
	sprintf(topic, "/pension/hamster/%d/wheels", hamsterID);

	char payload[15];
	memset(payload, 0, 15);
	sprintf(payload, "%d", wheels);

	return publish(client, topic, payload, 0);
}

/**
 * @brief main
 * 
 * @param argc 
 * @param argv 
 * @return int 
 */
int main(int argc, char **argv)
{
	int hamster_id = -1;
	char *owner = getenv("USER");
	char *hamster = "myhamster";
	char cmd;

	/*
	 * parse args:
	 */
	while ((cmd = getopt(argc, argv, "?p:i:s:o:n:hVv")) != -1)
	{
		switch (cmd)
		{
		case 'p':
		{
			char *end;
			port = strtoul(optarg, &end, 0);
			if (optarg == end)
			{
				printf("%s: Not a number: %s\n", argv[0], optarg);
				exit(EXIT_FAILURE);
			}
			break;
		}
		case 'i':
		{
			char *end;
			hamster_id = strtoul(optarg, &end, 0);
			if (optarg == end)
			{
				printf("%s: Not a number: %s\n", argv[0], optarg);
				exit(EXIT_FAILURE);
			}
			break;
		}
		case 's':
			ipaddr = optarg;
			break;
		case 'v':
			doEncrypt = 1;
			break;
		case 'V':
			doAuthenticate = 1;
			doEncrypt = 1;
			break;
		case 'h':
		case '?':
			rtfm(argv);
			return EXIT_SUCCESS;
			break;
		case 'o':
		{
			int l = strlen(optarg);
			if (l > 0 && l < HMSTR_MAX_NAME)
			{
				owner = optarg;
			}
			else
			{
				printf("%s: owner name empty or too long: %s\n", argv[0], optarg);
				exit(EXIT_FAILURE);
			}
			break;
		}
		case 'n':
		{
			int l = strlen(optarg);
			if (l > 0 && l < HMSTR_MAX_NAME)
			{
				hamster = optarg;
			}
			else
			{
				printf("%s: owner name empty or too long: %s\n", argv[0], optarg);
				exit(EXIT_FAILURE);
			}
			break;
		}
		}
	}

	if (hamster_id == -1)
	{
		hamster_id = make_hash(owner, hamster);

		debug(("Calling: hamster_id = make_hash(\"%s\", \"%s\");", owner, hamster));
	}
	sprintf(clientid, "%d", hamster_id);
	printf("** Using ID %d for %s's Hamster %s **\n", hamster_id, owner, hamster);

	/*
	 * connect to MQTT broker
	 *
	 */

	MQTTClient client;

	time_t startTime, endTime;
	int wheels;
	int state;
	char room;
	char URI[64];

	MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	int rc;
	MQTTClient_SSLOptions ssl_options = MQTTClient_SSLOptions_initializer;
	conn_opts.ssl = &ssl_options;

	// Für Verchlüsselungen Einstellungen anpassen
	if (doEncrypt)
	{
		ssl_options.trustStore = "certs/mqtt_ca.crt";
		ssl_options.enableServerCertAuth = true;
		sprintf(URI, "ssl://%s:%d", ipaddr, 8883);
	}
	if (doAuthenticate)
	{
		ssl_options.privateKey = "certs/keys/private.pem";
		ssl_options.keyStore = "certs/keys/certificate.crt";
		sprintf(URI, "ssl://%s:%d", ipaddr, 8884);
	}
	if (!doEncrypt && !doAuthenticate)
	{
		sprintf(URI, "tcp://%s:%d", ipaddr, port);
	}

	// Client erstellen und connecten
	MQTTClient_create(&client, URI, clientid,
					  MQTTCLIENT_PERSISTENCE_NONE, NULL);

	conn_opts.keepAliveInterval = 20;
	conn_opts.cleansession = 1;
	conn_opts.username = "hamster";

	MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

	if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
	{
		printf("Failed to connect, return code %d\n", rc);
		exit(EXIT_FAILURE);
	}

	// Topic Namen erstellen und anschließend subscriben
	char topic1[40];
	char topic2[40];
	sprintf(topic1, "/pension/hamster/%d/fondle", hamster_id);
	sprintf(topic2, "/pension/hamster/%d/punish", hamster_id);

	MQTTClient_subscribe(client, topic1, 1);
	MQTTClient_subscribe(client, topic2, 1);
	MQTTClient_subscribe(client, "/pension/room/#", 2);

	/* Hier ist was zu tun: */

	/*
	 * Get admission time, start RUNNING in room A
	 */
	time(&startTime);
	wheels = 0;
	state = RUNNING;
	room = 'A';
	/*
	 * publish our hamster ID on /pension/livestock:
	 */

	publish(&client, "/pension/livestock", clientid, 2);

	/* Hier ist was zu tun: */

	publishRoom(&client, hamster_id, "A");
	/*
	 * Main Command loop
	 */
	do
	{
		cli_commands();
		cmd = getchar();
		flushinput();
		switch (cmd)
		{
		case 'A': /* going to room A */
			publishRoom(&client, hamster_id, "A");
			room = 'A';

			break;
		case 'B': /* going to room B */
			publishRoom(&client, hamster_id, "B");
			room = 'B';

			break;
		case 'C': /* going to room C */
			publishRoom(&client, hamster_id, "C");
			room = 'C';

			break;
		case 'D': /* going to room D */
			publishRoom(&client, hamster_id, "D");
			room = 'D';

			break;
		case 'r': /* change to "RUNNING" state */
			if (state == RUNNING)
				break;
			time(&startTime);
			state = RUNNING;
			publishState(&client, hamster_id, "RUNNING");

			break;
		case 's': /* change to "SLEEPING" state */
			if (state == RUNNING)
			{
				time(&endTime);
				wheels += calculateWheels(startTime, endTime);
				publishWheels(&client, hamster_id, wheels);
			}
			state = SLEEPING;

			publishState(&client, hamster_id, "SLEEPING");

			break;
		case 'e': /* change to "EATING" state */
			if (state == RUNNING)
			{
				time(&endTime);
				wheels += calculateWheels(startTime, endTime);
				publishWheels(&client, hamster_id, wheels);
			}
			state = EATING;

			publishState(&client, hamster_id, "EATING");

			break;
		case 'm': /* change to "MATEING" state */
			if (state == RUNNING)
			{
				time(&endTime);
				wheels += calculateWheels(startTime, endTime);
				publishWheels(&client, hamster_id, wheels);
			}
			state = MATING;

			publishState(&client, hamster_id, "MATEING");

			break;
		case 'q':
			printf("quit\n");
			break;
		default:
			printf("unknown command\n\n");
		}
	} while (cmd != EOF && cmd != 'q');

	// Falls es später noch benutzt wird (erstmall vermeiden von compiler Warnungen)
	(void)room;

	MQTTClient_disconnect(client, 10000);
	MQTTClient_destroy(&client);
	return 0;
}
