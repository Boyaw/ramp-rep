
from ramp_server import Partition, RAMPAlgorithm
from ramp_client import Client
from random import sample, random, choice
from string import ascii_uppercase
from threading import Semaphore, Thread

# RAMPAlgorithm.{Fast, Small, Hybrid}
ALGORITHM = RAMPAlgorithm.Fast

NUM_PARTITIONS = 5
NUM_CLIENTS = 5

NUM_TXNS = 1000
READ_PROPORTION = .5
TXN_LENGTH = 4
NUM_KEYS = 100

KEYS = [str(i) for i in range(0, NUM_KEYS)]
# Create 5 partition objects in a list
PARTITIONS = [Partition() for _ in range(0, NUM_PARTITIONS)]

request_sem = Semaphore(NUM_TXNS)
finished_sem = Semaphore()

# generate a random string containing 6 uppercase letters, e.g. EKNLPI
def random_string():
    return ''.join(choice(ascii_uppercase) for _ in range(6))

def run_client(client):
    # request_sem.acquire(False) means "no block"
    while(request_sem.acquire(False)):
        # each transaction contains the same length of keys
        txn_keys = sample(KEYS, TXN_LENGTH)

        # with 0.5 probability this transaction is reading, otherwise it is writing
        if random() < READ_PROPORTION:
            client.get_all(txn_keys)
        else:
            kvps = {}
            value = random_string()
            for key in txn_keys:
                kvps[key] = value
            client.put_all(kvps)
        finished_sem.release()

for c_id in range(0, NUM_CLIENTS):
    client = Client(c_id, PARTITIONS, ALGORITHM)
    t = Thread(target=run_client, args=(client,))
    t.start()

# set to zero again TODO why do we need this?
# finished_sem.acquire(NUM_TXNS)

print "DONE!"

