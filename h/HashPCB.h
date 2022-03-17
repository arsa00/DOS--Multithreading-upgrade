#ifndef _hash_map_pcb_h_
#define _hash_map_pcb_h_

#include "pcb.h"

// separate chaining method for hash table; KEY in table must be an unsigned integer, because it represents index of array
class HashMapPCB
{
	struct Element
	{
		PCB* ptr_to_val;
		unsigned key;
		Element* next;
		Element* prev ;

		Element() : ptr_to_val(0), next(0), prev(0), key(0) { }
	};

public:
	const unsigned N;
	unsigned long size;

	HashMapPCB() : size(0), N(25)
	{
		entries = new Element *[N];
		for (int i = 0; i < N; i++)
			entries[i] = 0;
	}

	int add(PCB *val, unsigned key)
	{
		if (val == 0)
			return -1;	// ako je PCB == null, vracamo -1

		unsigned entry = hash(key);
		Element* new_el = new Element();	// proveri ima li mesta, ako ne vracamo -1
		/*if (new_el == 0)
			return -1;*/
		new_el->ptr_to_val = val;
		new_el->key = key;

		if (entries[entry] == 0) entries[entry] = new Element();	// inicijalizuj glavu liste ako vec nije

		if(entries[entry]->next != 0)
		{
			// ako lista u tom ulazu nije prazna, dopuni listu tog ulaza hash tabele
			new_el->next = entries[entry]->next;
			entries[entry]->next->prev = new_el;
		}
		// dodaj novi element na prvo mesto liste u tom ulazu hash tabele
		entries[entry]->next = new_el;
		new_el->prev = entries[entry];

		size++;
		return 0; // vraca 0 ako se dodavanje zavrsilo bez greske
	}

	PCB* get(unsigned key)
	{
		unsigned entry = hash(key);
		if (entries[entry] == 0 || entries[entry]->next == 0)
			return 0;	// ako nema tog elementa, tj. ako je prazan ulaz hash-a, vracamo null

		Element* ptr = entries[entry]->next;
		while (ptr)
		{
			if (ptr->key == key)
				break;
			ptr = ptr->next;
		}

		if (ptr != 0)
			return ptr->ptr_to_val;		// vracamo pronadjen element

		return 0;	// element nije pronadjen, vracamo null
	}

	int remove(unsigned key)
	{
		unsigned entry = hash(key);
		if (entries[entry] == 0 || entries[entry]->next == 0)
			return -1;	// ako nema tog elementa, tj. ako je prazan ulaz hash-a, vracamo -1

		Element* ptr = entries[entry]->next;
		while (ptr)
		{
			if (ptr->key == key)
				break;
			ptr = ptr->next;
		}

		if (ptr != 0)
		{
			ptr->prev->next = ptr->next;
			if(ptr->next != 0) ptr->next->prev = ptr->prev;
			delete ptr;		// brise samo prostor za element, ne brise vrednost (PCB) na koji pokazuje, to je na programeru!
			size--;
			return 0;		// uspesno izbrisan element, vracamo 0
		}

		return -1;	// element nije pronadjen, vracamo -1
	}

	int clear()
	{
		for (int i = 0; i < N; i++)
		{
			Element* ptr = entries[i];
			while (ptr)
			{
				Element* old = ptr;
				ptr = ptr->next;
				delete old;
			}
			entries[i] = 0;
		}

		size = 0;

		return 0;	// vrati 0 ako je uspesno
	}

private:
	Element** entries;

	unsigned hash(unsigned index)
	{
		return index % N;
	}
};

#endif
