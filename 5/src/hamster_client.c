/*
 * This is sample code generated by rpcgen.
 * These are only templates and you can use them
 * as a guideline for developing your own functions.
 */

#include "hamster.h"
#include "../include/hamsterlib.h"

CLIENT *clnt;

void hmstr_rpc_init(char *hostname) {
	clnt = clnt_create (hostname, HAMSTER_RPC, HAMSTER, "udp");
	if (clnt == NULL) {
		clnt_pcreateerror (hostname);
		exit (1);
	}
}

void hmstr_rpc_terminate(void) {
	clnt_destroy (clnt);
}

/**
 * @ingroup libfuncs
 * @brief	Put a new hamster into the hamstercare institute
 * 
 * This function makes a new entry into the hamster database. It returns
 * a unique ID by which the hamster can later be referenced. A Hamster
 * may be given a number of treats to feed.
 * 
 * 
 * @return If successful: Unique ID (always >= 0) of the new entry
 * @return On error: Error code (always < 0)
 * 
 */
int32_t hmstr_new(const char* owner_name, const char* hamster_name, 
					uint16_t treats) {
	struct hamster_new_arg new_arg;
	memcpy(new_arg.owner, owner_name, 32);
	memcpy(new_arg.hamster, hamster_name, 32);
	new_arg.treats = treats;
	int* result = hamster_new_1(&new_arg, clnt);
	if (result == (int *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	return *result;
}

/**
 * @ingroup libfuncs
 * @brief	Find hamster in the hamstercare institute
 * 
 * This function locates an entry in the hamster database. It returns
 * a unique ID by which the hamster can be referenced. A Hamster is
 * uniquely identified by the combination of the owner's name and the
 * hamster's name.
 * 
 * @return If successful: Unique ID (always >= 0) of the entry
 * @return On error: Error code (always < 0)
 * 
 */
int32_t hmstr_lookup(const char* owner_name, const char* hamster_name) {
	struct hamster_lookup_arg lookup_arg;
	memcpy(lookup_arg.owner, owner_name, 32);
	memcpy(lookup_arg.hamster, hamster_name, 32);
	int* result = hamster_lookup_1(&lookup_arg, clnt);
	if (result == (int *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	return *result;
}

/**
 * @ingroup libfuncs
 * @brief	Get a directory of entries in the database
 * 
 * This function enables a "wildcard search" of the database. It delivers
 * UIDs of matching entries. The caller may specify an owner name or a
 * hamster name, thus specifying a particular entry (in this case the
 * function is similar to hmstr_lookup()). However, it is also possible
 * to specify __only__ an owner name or __only__ a hamster name by passing
 * a NULL value for the name that should not be specified. In this
 * case, the function delivers UIDs of __all__ entries matching the
 * specified name. If both names are passed as NULL, the function delivers
 * the UIDs of __all__ entries in the database.
 * 
 * The function delivers __one__ __UID__ __per__ __call__. In order to
 * keep the context across calls, the caller must pass a pointer to an
 * int variable. This variable (__not__ the pointer!) must be initialized
 * with -1 before making the first call.
 * 
 * @return If successful: Unique ID (always >= 0) of the next matching entry
 * @return On error: Error code (always < 0)
 */
int32_t hmstr_directory(int32_t* fdptr,	const char* owner_name,
						const char* hamster_name) {
	struct hamster_directory_arg directory_arg;
	directory_arg.fdptr = *fdptr;
	if (owner_name == NULL) {
		memset(directory_arg.owner, 0, 32);
	} else {
		memcpy(directory_arg.owner, owner_name, 32);
	}
	if (hamster_name == NULL) {
		memset(directory_arg.hamster, 0, 32);
	} else {
		memcpy(directory_arg.hamster, hamster_name, 32);
	}

	struct hamster_directory_ret* result = hamster_directory_1(&directory_arg, clnt);
	if (result == (hamster_directory_ret *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	*fdptr = result->fdptr;
	return result->res;
}

/**
 * @ingroup libfuncs
 * @brief	How is my hamster doing?
 * 
 * This function checks upon hamster (at a cost!) identified by ID. It returns
 * the hamster's state in the given data structure.
 * 
 * @return If successful: 0, state of hamster stored in state struct
 * @return On error: Error code (always < 0)
 * 
 */
int32_t hmstr_howsdoing(int32_t ID, struct hmstr_state* state) {
	
	struct hamster_howsdoing_ret* result = hamster_howsdoing_1(&ID, clnt);
	if (result == (hamster_howsdoing_ret *) NULL) {
		clnt_perror (clnt, "call failed");
	}

	state->cost = result->cost;
	state->rounds = result->rounds;
	state->treats_left = result->treats_left;
	
	return result->res;
}

/**
 * @ingroup libfuncs
 * @brief	Get contents of an entry in the database
 * 
 * This function delivers details of a hamster identified by UID. The
 * price is __not__ changed by a call to this function.
 * 
 * @return If successful: number of treats left in hamster' store
 * @return On error: Error code (always < 0)
 */
int32_t hmstr_readentry(int32_t ID, char* owner, char* name, int16_t* price) {
	struct hamster_readentry_ret* result = hamster_readentry_1(&ID, clnt);
	if (result == (hamster_readentry_ret *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	
	memcpy(owner, result->owner, 32);
	memcpy(name, result->hamster, 32);
	*price = result->price;
	
	return result->res;
}

/**
 * @ingroup libfuncs
 * @brief	Give treats to my hamster
 * 
 * This function gives treats to the hamster identified by ID. The
 * Hamster's stock of treats will be used up first. If stock is
 * insufficient, more treats will be dispensed (at a cost!) and the
 * function returns a benign error.
 * 
 * @return If successful: number of treats left in stock (always >=0)
 * @return On error: Error code (always < 0)
 * 
 */
int32_t hmstr_givetreats(int32_t ID, uint16_t treats) {
	struct hamster_givetreats_arg givetreats_arg;
	givetreats_arg.ID = ID;
	givetreats_arg.treats = treats;

	int* result = hamster_givetreats_1(&givetreats_arg, clnt);
	if (result == (int *) NULL) {
		clnt_perror (clnt, "call failed");
	}
	
	return *result;
}

/**
 * @ingroup libfuncs
 * @brief	Collect all my hamsters and pay the bill
 * 
 * This function collects (i.e. deletes from the database) all
 * hamsters owned by the specified owner and sums up all their
 * expenses to produce a final bill.
 * 
 * @return If successful: number of euros to pay
 * @return On error: Error code (always < 0)
 * 
 */
int32_t hmstr_collect(const char* owner_name) {
	struct hamster_collect_arg collect_arg;
	memcpy(collect_arg.owner, owner_name, 32);

	int* result = hamster_collect_1(&collect_arg, clnt);
	if (result == (int *) NULL) {
		clnt_perror (clnt, "call failed");
	}

	return *result;
}

