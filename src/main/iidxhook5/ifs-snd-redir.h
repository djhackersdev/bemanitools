#ifndef IIDXHOOK5_IFS_SND_REDIR_H
#define IIDXHOOK5_IFS_SND_REDIR_H

/**
 * Redirect any reads from the ifs files in data/imagefs that contain
 * sound data to local files in data/sound. Required to enable bemanitools
 * monitor-check feature to trap open calls to all sound data and apply
 * different monitor refresh values.
 */
void iidxhook5_ifs_snd_redir_init();

#endif
