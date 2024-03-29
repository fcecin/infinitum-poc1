This is a fork of the master branch of the Bitcoin Core project. (https://bitcoincore.org, bitcoin/bitcoin on GitHub) 

The license for the code is the same MIT license.

"Infinitum" is a simple modification of Bitcoin to turn it into a transferable hashcash system. As in the original Hashcash anti-spam system, the value of generated proof-of-work tokens is directly proportional to the computing power spent. Infinitum solves double-spending for tokens generated by proof-of-work with its concept of blocks and block rewards, and all we have to change to make that system into a double-spending-protected transferable hashcash system is to make the reward for a block be proportional to the difficulty of the block.

In addition to this simple change, I'm trying to implement yearly checkpointing of the block database. As soon as there is a whole year "Y" worth of blocks that is confirmed by another whole year "Y+1" of transactions on top of it, year "Y" has all of its block data deleted and replaced by a (new) root block that is essentially a huge list of unspent balances. With this (and some other tweaks) the system can rid itself of all transaction history that is more than two years old.

Here's some more discussion on the hashcash and checkpointing tweaks: https://medium.com/@fcecin/democratic-digital-money-system-plan-final-a79dcea25c51
