/**
 * \file MyQueue.h
 * \brief Fichier header de la classe MyQueue
 * \author Paul-Antoine Boutet
 * \version 0.0.1
 * \date 13/09/2017
 */
 
#ifndef MY_QUEUE_H
#define MY_QUEUE_H

#include <Arduino.h>

/**
 * /class	MyQueue
 * /brief	Classe qui permet de gérer une liste de type de type FIFO
 */
template<typename T>
class MyQueue {

	public :
		MyQueue( int maxSize=2 );
		~MyQueue();

		void push( T item );
		T pop();
		T peek( int index = 0 );
		int count();
		bool isEmpty();
		bool isFull();
		void resize( int newMaxSize );

		void print();

	private :

		T * contents;		// Tableau contenant les elements
		int contentsMaxSize;	// Taille max du tableau

		int head;		// Index pour la tête de liste
		int tail;		// Index pour la queue de liste
		int items;		// Nombre d'élements dans la liste
		
};

/**
 * \brief Constructeur de la classe
 *
 * \param maxSize : (int) description du paramètre d'entrée
 */
template<typename T>
MyQueue<T>::MyQueue( int maxSize ){
	contents = (T *) malloc (sizeof (T) * maxSize);
	contentsMaxSize = maxSize;
	head = 0;
	tail = 0;
	items = 0;
}

/**
 * \brief Destructeur de la classe
 */
template<typename T>
MyQueue<T>::~MyQueue(){
	free( contents );
}

/**
 * \fn void push( T item)
 * \brief Fonction qui ajoute un élément à la pile
 *
 * \param item : (T) element à ajouter
 */
template<typename T>
void MyQueue<T>::push( T item ){
	
	if( items != 0 ) 					// Si la liste n'est pas vide
		head = (head+1)% contentsMaxSize;		// On incrémente la tête de liste

	contents[head] = item;					// On ajoute l'element

	if( items == contentsMaxSize ) 				// Si la liste est pleine
		tail = (tail+1)% contentsMaxSize;		// On incrémente la queue de liste
	else 
		items++;					// On incrémente le nombre d'éléments dans la liste

}

/**
 * \fn T pop()
 * \brief Fonction qui retourne le plus viel element et qui le supprime
 *
 * \return item : (T) plus viel element
 */
template<typename T>
T MyQueue<T>::pop(){

	if( items == 0 ) exit("pop pas possible : La liste est vide");

	int indexBuf = tail;					// On enregistre l'index de l'élement à retourner

	if( items > 1 ) tail = (tail+1)% contentsMaxSize;	// On met à jour le nouvel index de queue
	items--;						// On indique qu'il y a un element en moins

	return contents[indexBuf];

}

/**
 * \fn int count()
 * \brief Fonction qui retourne le nombre d'élements dans la liste
 *
 * \return items : (int) nombre d'elements dans la liste
 */
template<typename T>
int MyQueue<T>::count(){
	return items;
}

/**
 * \fn T peek()
 * \brief Fonction qui retourne l'élément à l'index désiré sans le supprimer
 *
 * L'index 0 correspond au dernier élément ajouté, 1 à l'avant dernier, etc...
 *
 * \param index : (int) index, par défault = 0 pour indiquer le plus viel element
 *
 * \return item : (T) Element désiré
 */
template<typename T>
T MyQueue<T>::peek( int index ){

	if( index > contentsMaxSize )
		exit( "L'élément désiré n'existe pas" );

	int trueIndex = head - index;				// On cherche l'index dans le tableau à partir de l'index de tête
	if( trueIndex < 0 ) trueIndex += contentsMaxSize;	// Si l'index calculé est négatif on recherche à partir de l'autre coté du tableau

	return contents[trueIndex];
}


/**
 * \fn bool isEmpty()
 * \brief Fonction qui indique si la table est vide
 *
 * \return bool
 */
template<typename T>
bool MyQueue<T>::isEmpty(){
	return items == 0;
}

/**
 * \fn bool isFull()
 * \brief Fonction qui indique si la table est pleine
 *
 * \return bool
 */
template<typename T>
bool MyQueue<T>::isFull(){
	return items == contentsMaxSize;
}

/**
 * \fn void resize()
 * \brief Fonction qui met a jour la capacité max du tableau
 *
 * \param newMaxSize : (int) Nouvelle taille du tableau
 */
template<typename T>
void MyQueue<T>::resize( int newMaxSize ){

	if( newMaxSize < items )
		exit("Redimmensionnement impossible, cela entrainerait une perte de données");

	T * temp = (T *) malloc( sizeof(T)* newMaxSize );	// Création d'un nouveau tableau

	for( int i=0; i<items; i++){				// Copie des elements dans le nouveau tableau
		temp[i] = contents[ (tail + i)%contentsMaxSize ];
	}

	free( contents );					// Liberation mémoire de l'ancien tableau

	contents = temp;				

	contentsMaxSize = newMaxSize;
	head = items -1;
	tail = 0;

}



template<typename T>
void MyQueue<T>::print(){
	int i;
	for ( i = 0;  i < contentsMaxSize;  i++) {
		Serial.print("\t[");
		Serial.print(i);
		Serial.print("]");
		if( head == i ) Serial.print("h");
		if( tail == i ) Serial.print("t");
		Serial.print(":\t");
		Serial.print( contents[i] );
	}
	Serial.println();
}

#endif
