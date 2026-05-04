/*
 * unp_prod_cons.cc
 *
 *  Created on: 11.10.2016
 *      Author: kudr
 */

#include <cstddef>
#include "unp_assert.h"

#include "unp_prod_cons.h"

namespace UNP {
namespace PROD_CONS {

Fifo::~Fifo() {
	delete prod;
	delete cons;
	
	while(!vacant.empty()) {
		Task *t = vacant.front();
		vacant.pop();
		delete t;
	}
	
	while(!fifo.empty()) {
		Task *t = fifo.front();
		fifo.pop();
		delete t;
	}
}

void Fifo::VacateFifo() {
	while(!fifo.empty()) {
		Task *t = fifo.front();
		fifo.pop();
		vacant.push(t);
	}
}

void Fifo::VacateAll() {
	while(!fifo.empty()) {
		Task *t = fifo.front();
		fifo.pop();
		vacant.push(t);
	}
	
	if(prod) {
		vacant.push(prod);
		prod = NULL;
	}
	
	if(cons) {
		vacant.push(cons);
		cons = NULL;
	}
}

Task* Fifo::ProducerTake() {
	UNP_Assert(!prod);
	
	if(vacant.empty())
		return NULL;
	else {
		prod = vacant.front();
		vacant.pop();
		return prod;
	}
}

void Fifo::ProducerPut(bool toProcessFifo) {
	UNP_Assert(prod);
	
	(toProcessFifo ? fifo : vacant).push(prod);
	prod = NULL;
}

Task* Fifo::ConsumerTake() {
	UNP_Assert(!cons);
	
	if(fifo.empty())
		return NULL;
	else {
		cons = fifo.front();
		fifo.pop();
		return cons;
	}
}

void Fifo::ConsumerPut() {
	UNP_Assert(cons);
	
	vacant.push(cons);
	cons = NULL;
}

}
}
