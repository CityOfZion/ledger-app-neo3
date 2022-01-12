#include "../common/buffer.h"
#include "types.h"

void try_parse_transfer_script(buffer_t *script, transaction_t *tx);

void try_parse_vote_script(buffer_t *script, transaction_t *tx);