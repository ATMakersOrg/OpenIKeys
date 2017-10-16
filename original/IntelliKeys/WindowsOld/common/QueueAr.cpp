#ifndef _QueueAr_CPP_
#define _QueueAr_CPP_

#include "IKCommon.h"

        #include "QueueAr.h"

        /**
         * Construct the queue.
         */
        template <class Object>
        Queue<Object>::Queue( int capacity ) : theArray( capacity )
        {
            makeEmpty( );
			locked = false;
        }

        /**
         * Test if the queue is logically empty.
         * Return true if empty, false, otherwise.
         */
        template <class Object>
        bool Queue<Object>::isEmpty( ) const
        {
            return currentSize == 0;
        }

        /**
         * Test if the queue is logically full.
         * Return true if full, false, otherwise.
         */
        template <class Object>
        bool Queue<Object>::isFull( ) const
        {
            return currentSize == theArray.size( );
        }
        
        template <class Object>
        int Queue<Object>::GetSize( ) const
        {
            return currentSize;
        }

        /**
         * Make the queue logically empty.
         */
        template <class Object>
        void Queue<Object>::makeEmpty( )
        {
			Lock();
            currentSize = 0;
            front = 0;
            back = -1;
			Unlock();
        }

        /**
         * Get the least recently inserted item in the queue.
         * Return the least recently inserted item in the queue
         * or throw Underflow if empty.
         */
        template <class Object>
        bool Queue<Object>::getFront( Object & x ) const
        {
            if( isEmpty( ) )
				return false;

			x = theArray[ front ];
			return true;
        }

        /**
         * 
         *   Remove the entry at the given index.
         */
        template <class Object>
        bool Queue<Object>::removeAt( int ndx )
		{
            if( isEmpty( ) )
				return false;

			//  todo
			return false;
		}

        /**
         * 
         *   Set a new value for the earliest entry, effectively throwing away
		 *   everything before that point.
         */
        template <class Object>
		bool Queue<Object>::newBack ( int ndx )
		{
            if( isEmpty( ) )
				return false;

			if (ndx>=currentSize)
				return false;

			int i = front + ndx;
			if (i>=theArray.size())
				i = i - theArray.size();

			back = i;
			currentSize = ndx + 1;

			return true;
		}


        /**
         *   Get the object at a given index in the queue.
         */
        template <class Object>
		bool Queue<Object>::getAt( Object & x, int ndx ) const
		{
            if( isEmpty( ) )
				return false;

			if (ndx>=currentSize)
				return false;

			int i = front + ndx;
			if (i>=theArray.size())
				i = i - theArray.size();

			x = theArray[ i ];
			return true;

		}

        template <class Object>
        bool  Queue<Object>::removeFront( void )
        {
            if( isEmpty( ) )
				return false;
 
			Lock();
            currentSize--;
            increment( front );
			Unlock();
 			return true;
        }
        /**
         * Return and remove the least recently inserted item from the queue.
         * Throw Underflow if empty.
         */
        template <class Object>
         bool Queue<Object>::dequeue( Object & x )
        {
            if( isEmpty( ) )
				return false;

			Lock();
			bool b = dequeueNoLock(x);
			Unlock();
 			return b;
        }

        template <class Object>
         bool Queue<Object>::dequeueNoLock( Object & x )
        {
            if( isEmpty( ) )
				return false;

            currentSize--;
			x = theArray[ front ];
            increment( front );
 			return true;
        }

        /**
         * Insert x into the queue.
         * Throw Overflow if queue is full.
         */
        template <class Object>
        bool Queue<Object>::enqueue( const Object & x )
        {
            if( isFull( ) )
				return false;
 
			Lock();
            increment( back );
            theArray[ back ] = x;
            currentSize++;
			Unlock();
			return true;
        }

        /**
         * Internal method to increment x with wraparound.
         */
        template <class Object>
        void Queue<Object>::increment( int & x )
        {
            if( ++x == theArray.size( ) )
                x = 0;
        }

        /**
         * Internal method to lock the queue
         */
        template <class Object>
        void Queue<Object>::Lock()
        {
			//  spin waiting for unlocked
			while(locked)
			{
			}

			//  lock it
			locked = true;
        }

        /**
         * Internal method to unlock the queue
         */
        template <class Object>
        void Queue<Object>::Unlock()
        {
			locked = false;
        }

#endif //  _QueueAr_CPP_
