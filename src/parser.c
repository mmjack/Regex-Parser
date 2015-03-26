/*
 * parser.c
 *
 *  Created on: 30 Nov 2014
 *      Author: blake
 */
#include <stdlib.h>
#include <stdio.h>
#include "parser.h"
#include "nfa_fragment.h"
#include "stack.h"
#include "nfa.h"
#include "infix.h"

nfa_fragment* basicFragment(nfa_state* state) {
	nfa_fragment* frag = nfaFragmentCreate();
	frag->start = state;
	nfaFragmentAddTail(frag, state);
	return frag;
}

nfa_state* createState(unsigned short condition, regex* regex) {
	nfa_state* state = nfaStateCreate(condition, NULL, NULL);
	nfaListAdd(&regex->stateList, state);
	return state;
}

bool regexParse(regex* regexStructure, char const* input) {
	char* infixWithConcatenations = infixInsertExplicitConcatenation(input);
	printf("Infix with concatenations inserted %s\n", infixWithConcatenations);

	char* postFix = infixToPostfix(infixWithConcatenations);
	printf("Infix Conversion: %s\n", postFix);
	free(infixWithConcatenations);
/*
	bool result = regexParse(regexStructure, postFix);
	free(postFix);
	return result;*/
	return false;
}

bool regexParsePostfix(regex* regexStructure, char const* input) {

	generic_stack* stateStack = stackAllocate(sizeof(nfa_fragment*));
	nfa_state *state, *state2;
	nfa_fragment *t1, *t2, *t3;

	nfaListAllocate(&regexStructure->stateList, 1000);

	for (; *input; input++) {
		switch (*input) {
		case '&':
			//Pop the two sides of the concat
			stackPop(stateStack, &t2);
			stackPop(stateStack, &t1);

			//Patch the tail states on t1 to the start state on t2
			nfaFragmentPatch(t1, t2->start);

			//Create a new fragment using t2's tail states and t1's start states (The concat of both)
			t3 = nfaFragmentCreate();
			t3->start = t1->start;
			nfaFragmentFillTails(t3, t2);

			//Free the two popped fragment
			nfaFragmentFree(t1);
			nfaFragmentFree(t2);

			//Push the new one
			stackPush(stateStack, &t3);
			break;
		case '|':
			stackPop(stateStack, &t2);
			stackPop(stateStack, &t1);

			state = createState(256, regexStructure);
			state->path = t1->start;
			state->alternative = t2->start;

			t3 = nfaFragmentCreate();
			t3->start = state;

			nfaFragmentFillTails(t3, t1);
			nfaFragmentFillTails(t3, t2);

			nfaFragmentFree(t1);
			nfaFragmentFree(t2);

			stackPush(stateStack, &t3);
			break;
		case '+':
			stackPop(stateStack, &t1);

			state = createState(256, regexStructure);
			state->path = NULL;
			state->alternative = t1->start;

			nfaFragmentPatch(t1, state);

			t3 = nfaFragmentCreate();
			t3->start = t1->start;
			nfaFragmentAddTail(t3, state);

			nfaFragmentFree(t1);

			stackPush(stateStack, &t3);
			break;
		case '*':
			stackPop(stateStack, &t1);

			state2 = createState(256, regexStructure);
			state2->path = NULL;
			state2->alternative = t1->start;

			state = createState(256, regexStructure);
			state->path = NULL;
			state->alternative = state2;

			nfaFragmentPatch(t1, state2);

			t3 = nfaFragmentCreate();
			t3->start = state;

			nfaFragmentAddTail(t3, state);
			nfaFragmentAddTail(t3, state2);

			nfaFragmentFree(t1);

			stackPush(stateStack, &t3);
			break;
		case '?':
			stackPop(stateStack, &t1);

			state = createState(256, regexStructure);
			state->path = NULL;
			state->alternative = t1->start;

			t3 = nfaFragmentCreate();
			t3->start = state;
			nfaFragmentAddTail(t3, state);
			nfaFragmentFillTails(t3, t1);

			nfaFragmentFree(t1);

			stackPush(stateStack, &t3);
			break;
		case '.':
			state = createState(258, regexStructure);
			t3 = basicFragment(state);
			stackPush(stateStack, &t3);
			break;
		default:
			state = createState(*input, regexStructure);
			t3 = basicFragment(state);
			stackPush(stateStack, &t3);
			break;
		}
	}

	//Concatenate a accepting at the end
	state = createState(257, regexStructure);
	t3 = basicFragment(state);

	stackPop(stateStack, &t1);
	nfaFragmentPatch(t1, state);

	t3->start = t1->start;
	nfaFragmentAddTail(t3, state);

	nfaFragmentFree(t1);
	stackPush(stateStack, &t3);

	stackPop(stateStack, &t1);
	regexStructure->start = t1->start;
	nfaFragmentFree(t1);

	while (!stackEmpty(stateStack)) {
		stackPop(stateStack, &t1);
		printf("WARN: Stuff left in stack. Your regular expression sucks.\n");
		nfaFragmentFree(t1);
	}

	stackFree(stateStack);
	return true;
}

void regexFree(regex* regexStructure) {

	unsigned int i;

	for (i = 0; i < regexStructure->stateList.currentSize; i++) {
		nfaStateFree(regexStructure->stateList.states[i]);
	}

	nfaListFree(&regexStructure->stateList);
}
