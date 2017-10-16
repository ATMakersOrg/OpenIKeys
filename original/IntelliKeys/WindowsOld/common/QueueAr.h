        #ifndef _QUEUEAR_H
        #define _QUEUEAR_H

        #include "vector.h"

        // Queue class -- array implementation
        //
        // CONSTRUCTION: with or without a capacity; default is 10
        //
        // ******************PUBLIC OPERATIONS*********************
        // bool enqueue( x )			 --> Insert x
        // bool dequeue( Object & x )    --> Return and remove least recently inserted item
        // bool getFront( Object & x )   --> Return least recently inserted item
        // bool isEmpty( )				 --> Return true if empty; else false
        // bool isFull( )				 --> Return true if full; else false
        // void makeEmpty( )			 --> Remove all items
        // ******************ERRORS********************************

        template <class Object>
        class Queue
        {
          public:
            explicit Queue( int capacity = 3000 );

            bool isEmpty( ) const;
            bool isFull( ) const;
            int GetSize() const;
            bool getFront( Object & x ) const;
            bool removeFront( void );
            bool removeAt( int ndx );
            bool getAt( Object & x, int ndx ) const;
			bool newBack ( int ndx );
         
            void makeEmpty( );
            bool dequeue( Object & x );
            bool dequeueNoLock( Object & x );
            bool enqueue( const Object & x );

			void Lock();
			void Unlock();

          private:
            vector<Object> theArray;
            int            currentSize;
            int            front;
            int            back;

            void increment( int & x );

			bool locked;

        };

        #include "QueueAr.cpp"

        #endif
