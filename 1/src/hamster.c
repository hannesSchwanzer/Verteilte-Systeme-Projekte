/**
 * @file	hamster.c
 * @brief	Hamsterverwaltungsprogramm
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>
#include <errno.h>


#include "hamsterlib.h"

/**
 * @brief Kurze Hilfe ausgeben
 */
static void rtfm(char* argv[])
{
	printf("Usage: %s {<Option>} <param1> {<param2>}\n", argv[0]);
	printf("Function: Hamster management\n");
	printf("Optionen:\n");
	printf("     -l {<owner>}                    - show current list of hamsters\n");
	printf("     -n <owner> <hamster> [<treats>] - add new hamster\n");
	printf("     -f <owner> <hamster> <treats>   - feed treats to hamster\n");
	printf("     -s <owner> <hamster>            - how is my hamster doing?\n");
	printf("     -b <owner>                      - the bill please!\n");
}
/**
 * @brief Check if an error occurred
 * This function checks if an error occurred, prints an error message
 * and exits the program
 * @param errNo 
 * @param functionName 
 * 
 */
void handleError(int32_t errNo, const char* functionName) 
{
	//Kein Fehler
	if (errNo >= 0) return;

	//Fehlerbehandlung
	switch (errNo)
	{
	case -1:
		printf("In function %s: Error: the specified name is too long\n", functionName);
		exit(EXIT_FAILURE);
		break;
	case -2:
		printf("In function %s: Error: a hamster by that owner/name already exists\n", functionName);
		exit(EXIT_FAILURE);
		break;
	case -3:
		printf("No hamsters matching criteria found\n");
		exit(EXIT_FAILURE);
		break;
	case -100:
		printf("Fehler -100: Storage Error.\n");
		exit(EXIT_FAILURE);
		break;
	case -101:
		printf("Fehler -101: Die Datenbank ist corrupted.\n");
		exit(EXIT_FAILURE);
		break;
	default:
		printf("Unkown error: %d\n", errNo);
		exit(EXIT_FAILURE);
		break;
	}
}
/**
 * @brief Prints all Hamster with matching criteria
 * If owner_name is NULL, this function prints owner, name price and treats of all hamsters
 * If a owner_name is given, it only prints the information for hamsters from the owner
 * 
 * @param owner_name 
 */
void printHamster(char* owner_name) 
{
	//Variablen fuer die noetigen Informationen
	char owner[HMSTR_MAX_NAME+1];
	char name[HMSTR_MAX_NAME+1];
	int16_t price;
	int32_t treats;	
	
	//erste id holen
	int32_t n = -1;
	int32_t id = hmstr_directory(&n, owner_name, NULL);
	handleError(id, "hmstr_directory()");

	printf("Owner\t\t\t\t\tName\t\t\t\tPrice\ttreats left\n");
	while (id>=0)
	{
		//Strings leeren
		memset(owner, 0, HMSTR_MAX_NAME+1);
		memset(name, 0, HMSTR_MAX_NAME+1);

		//Informationen von Hamster holen
		treats = hmstr_readentry(id, owner, name, &price);
		handleError(treats, "hmstr_readentry()");

		printf("| %-32s %-32s %8d €\t%-8d |\n", owner, name, price, treats);
		//nächster Hamster
		id = hmstr_directory(&n, owner_name, NULL);
	}

	//Fehlerbehandlung. -3 wird weggelassen, da es immer kommt wenn alle Hamster durchlaufen wurden
	if (id != -3) handleError(id, "hmstr_directory()");
}


/**
 * @brief Parses a number from a string
 * If the string is not a number, the function prints a
 * error message and exits the programm
 * @param str 
 * @return unsigned long : Parsed number from String
 */
unsigned long strToNum(const char* str)
{
	//Check if String is a Number
	for (int i = 0; i<strlen(str); i++)
	{
		if(str[i] < '0' || str[i] > '9')
		{
			printf("Please enter a valid number\n");
			exit(EXIT_FAILURE);
		}
	}
	 
	char* endptr;
	unsigned long num = strtoul(str, &endptr, 10);
	if (num == ULONG_MAX)
	{
		perror("Der angegebene Wert konnte in keine Zahl umgewandelt werden.");
		exit(EXIT_FAILURE);
	}
	return num;
}




/**
 * @brief Main program
 */
int main(int argc, char* argv[])
{

	//schauen ob genung Argumente vorhanden sind
	if (argc < 2) 
	{
    	rtfm(argv);
		exit(EXIT_FAILURE);
	}


	char* argument = argv[1];
	if (argument[0] != '-')
	{
    	rtfm(argv);
		exit(EXIT_FAILURE);
	}

	//Argumente anschauen
	switch (argument[1])
	{	
	case 'l':
	{
		switch (argc)
		{
		//Besitzer Name wurde mitgegeben
		case 3:
			printHamster(argv[2]);
			break;
		//Keine Besitzer Name
		case 2:
			printHamster(NULL);
			break;
		default:
			rtfm(argv);
			exit(EXIT_FAILURE);
		}
		break;
	} 
	case 'n':
	{
		
		if (argc == 4)
		{
			//Erstelle Hamster ohne Leckerlies
			handleError(
				hmstr_new(argv[2], argv[3], 0), "hmstr_new()");
			printf("Done!\n");
		}
		else if (argc == 5)
		{
			//Erstelle Hamster mit Leckerlies

			//Zahl parsen 
			unsigned long treats = strToNum(argv[4]);

			//Hamster erstellen
			handleError(
				hmstr_new(argv[2], argv[3], treats), "hmstr_new()");
			printf("Done!\n");
		}
		else {
			rtfm(argv);
		}
		break;
	}
	case 'f':
	{
		//Leckerlies an Hamster geben

		if (argc != 5)
		{
			rtfm(argv);
			exit(EXIT_FAILURE);
		}

		//Zahl parsen 
		unsigned long treats = strToNum(argv[4]);

		//Id von Hamster suchen
		int32_t id = hmstr_lookup(argv[2], argv[3]);
		handleError(id, "hmstr_lookup()");

		//Leckerlies geben
		int trLeft = hmstr_givetreats(id, treats);
		handleError(trLeft, "hmstr_givetreats()");
		printf("Done! %d treats remaining in store\n", trLeft);
		break;
	}
	case 's':
	{	
		if (argc != 4)
		{
			rtfm(argv);
			exit(EXIT_FAILURE);
		}

		//Variablen fuer die noetigen Informationen
		struct hmstr_state state;
		char owner[HMSTR_MAX_NAME+1];
		char name[HMSTR_MAX_NAME+1];
		int16_t price;

		//Strings leeren (zur Sicherheit)
		memset(owner, 0, HMSTR_MAX_NAME+1);
		memset(name, 0, HMSTR_MAX_NAME+1);

		//Id finden
		int32_t id = hmstr_lookup(argv[2], argv[3]);
		handleError(id, "hmstr_lookup()");
		
		//Information von ID holen
		handleError(
			hmstr_readentry(id, owner, name, &price), "hmstr_readentry()");
		handleError(
			hmstr_howsdoing(id, &state), "hmstr_howsdoing()");
		
		printf("%s's hamster %s has done %d hamster wheel revolutions,\nand has %d treats left in store. Current price is %d €", owner, name, state.rounds, state.treats_left, state.cost);
		break;
	}
	case 'b':
	{
		if (argc != 3)
		{
			rtfm(argv);
			exit(EXIT_FAILURE);
		}

		//Preis ausgeben lassen
		int32_t sum = hmstr_collect(argv[2]);
		handleError(sum, "hmstr_collect()");
		printf("%s has to pay %d €\n", argv[2], sum);
		break;
	}
	default:
	{
		rtfm(argv);
		exit(EXIT_FAILURE);
		break;
	}
	}
	

	return EXIT_SUCCESS;
}
