

void btsdk_dllmain_bootstrap_process_attach()
{
    // TODO have the whole stuff to bootstrap independently of hook API
    // with logging setup etc.
    // provide stuff via recovering from command line args or env vars?
}

void btsdk_dllmain_bootstrap_process_detach()
{
    // TODO cleanup

    core_log_bt_fini();
}