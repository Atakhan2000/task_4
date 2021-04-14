#ifndef MEMORY_MANAGER_HEAD_H_2021_02_18
#define MEMORY_MANAGER_HEAD_H_2021_02_18

#include <iostream>
#include <cassert>
#include <stdexcept>
#include <cstring>

namespace lab618
{
    template <class T>
    class CMemoryManager
    {
    private:
        struct block
        {
            // Массив данных блока
            T* pdata;
            // Адрес следующего блока
            block *pnext;
            // Первая свободная ячейка
            int firstFreeIndex;
            // Число заполненных ячеек
            int usedCount;
            block(T* _pdata, block * _pnext, int _firstFreeIndex, int _usedCount)
            : pdata(_pdata), pnext(_pnext), firstFreeIndex(_firstFreeIndex),
			  usedCount(_usedCount)
            {
            };
        };
    public:
        class CException
        {
        public:
            CException()
            {
            }
        };

    public:
        CMemoryManager(int _default_block_size, bool isDeleteElementsOnDestruct = false)
        :m_blkSize(_default_block_size),
		 m_pBlocks(nullptr),
		 m_pCurrentBlk(nullptr),
		 m_isDeleteElementsOnDestruct(isDeleteElementsOnDestruct)
        {
        }

        virtual ~CMemoryManager()
        {
        	clear();
        }

        // Получить адрес нового элемента из менеджера
        T* newObject()
        {
            if (!m_pBlocks)
            {
                m_pBlocks = m_pCurrentBlk = newBlock();

                T* pEmptyPlace = m_pCurrentBlk->pdata + m_pCurrentBlk->firstFreeIndex;
                int nextFreeIndex = *reinterpret_cast<int*> (pEmptyPlace);
                T* pNewElement = new (pEmptyPlace) T;
                m_pCurrentBlk->firstFreeIndex = nextFreeIndex;
                m_pCurrentBlk->usedCount += 1;
                return pNewElement;
            }

            if (m_pCurrentBlk->firstFreeIndex != -1)
            {
                T* pEmptyPlace = m_pCurrentBlk->pdata + m_pCurrentBlk->firstFreeIndex;
                int nextFreeIndex = *reinterpret_cast<int*> (pEmptyPlace);
                T* pNewElement = new (pEmptyPlace) T;
                m_pCurrentBlk->firstFreeIndex = nextFreeIndex;
                m_pCurrentBlk->usedCount += 1;
                return pNewElement;
            }

            block* pFullBlock = m_pBlocks;
            block* pNotFullBlock = nullptr;
            while (pFullBlock)
            {
                if (pFullBlock->firstFreeIndex == -1)
                {
                    pFullBlock = pFullBlock->pnext;
                    continue;
                }
                pNotFullBlock = pFullBlock;
                break;
            }

            if (!pNotFullBlock)
            {
                pNotFullBlock = newBlock();
                m_pCurrentBlk->pnext = pNotFullBlock;
                m_pCurrentBlk = m_pCurrentBlk->pnext;
            }
            m_pCurrentBlk = pNotFullBlock;

            T* pEmptyPlace = m_pCurrentBlk->pdata + m_pCurrentBlk->firstFreeIndex;
            int nextFreeIndex = *reinterpret_cast<int*> (pEmptyPlace);
            T* pNewElement = new (pEmptyPlace) T;
            m_pCurrentBlk->firstFreeIndex = nextFreeIndex;
            m_pCurrentBlk->usedCount += 1;
            return pNewElement;
        }

        // Освободить элемент в менеджере
        bool deleteObject(T* p)
        {
            if (!m_pBlocks)
            {
                throw new CException();
            }

            block* pBlkWithDelPtr = m_pBlocks;
            while (pBlkWithDelPtr)
            {
                T* pLeft = pBlkWithDelPtr->pdata;
                T* pRight = pBlkWithDelPtr->pdata + m_blkSize;
                if (pLeft <= p && p < pRight)
                {
                    break;
                }
                pBlkWithDelPtr = pBlkWithDelPtr->pnext;
            }

            if (!pBlkWithDelPtr)
            {
                throw new CException();
            }

            p->~T();
            int num = p - pBlkWithDelPtr->pdata;
            new(reinterpret_cast<int*> (p)) int(pBlkWithDelPtr->firstFreeIndex);
            pBlkWithDelPtr->firstFreeIndex = num;
            pBlkWithDelPtr->usedCount -= 1;
        }

        // Очистка данных, зависит от m_isDeleteElementsOnDestruct
        void clear()
        {
        	if (nullptr == m_pBlocks)
        	{
        		return;
        	}
        	if (!m_isDeleteElementsOnDestruct && !areAllBlocksEmpty())
        	{
        		throw CException();
        	}
        	block* pcurr_blk = m_pBlocks;

        	m_pBlocks = nullptr;
        	m_pCurrentBlk = nullptr;

        	bool * pmask = new bool[m_blkSize];

            while (nullptr != pcurr_blk)
        	{
        		block* pnext = pcurr_blk->pnext;
        		deleteBlock(pcurr_blk, pmask);
        		pcurr_blk = pnext;
        	}
            delete[] pmask;
        }

        bool areAllBlocksEmpty() const
        {
        	block* pcurr_blk = m_pBlocks;
        	while ((nullptr != pcurr_blk) && (pcurr_blk->usedCount == 0))
        	{
        		pcurr_blk = pcurr_blk->pnext;
        	}
        	return nullptr == pcurr_blk;
        }

    private:

        // Создать новый блок данных. применяется в newObject
        block* newBlock()
        {
            char* pArr = ::new char[m_blkSize * sizeof(T)];
            T* pNewArr = reinterpret_cast<T*> (pArr);

            for(int i = 0; i < m_blkSize; ++i) {
                T* pNextElement = pNewArr + i;
                int next_empty_pos = (i == m_blkSize - 1) ? -1 : i + 1;
                new(reinterpret_cast<int*> (pNextElement)) int(next_empty_pos);
            }

            return new block(pNewArr, nullptr, 0, 0);
        }

        // Освободить память блока данных. Применяется в clear
        void deleteBlock(block *p, bool *pmask)
        {
        	//delete[] reinterpret_cast<char*>(p->pdata);
        	//delete p;
        	if (!m_isDeleteElementsOnDestruct)
        	{
        		delete[] reinterpret_cast<char*>(p->pdata);
        		delete p;

        		return;
        	}

        	for (int i = 0; i < m_blkSize; ++i)
        	{
        		pmask[i] = true;
        	}

        	T* pData = p->pdata;
        	int freeIndex = p->firstFreeIndex;
        	while (freeIndex != 1)
        	{
        		pmask[freeIndex] = false;
        		freeIndex = *(reinterpret_cast<int*>(pData + freeIndex));
        	}
        	for (int i = 0; i < m_blkSize; ++i)
        	{
        		if (pmask[i])
        		{
        			(pData +  i)->~T();
        		}
        	}

        	delete[] reinterpret_cast<char*>(p->pdata);

        	delete p;


        }

        // Размер блока
        int m_blkSize;
        // Начало списка блоков
        block* m_pBlocks;
        // Текущий блок
        block *m_pCurrentBlk;
        // Удалять ли элементы при освобождении
        bool m_isDeleteElementsOnDestruct;
    };
}; // namespace lab618

#endif // #define MEMORY_MANAGER_HEAD_H_2021_02_18
