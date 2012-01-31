isEmpty(INSTALLBASE): INSTALLBASE = $$(INSTALLBASE)
isEmpty(INSTALLBASE) {
    INSTALLBASE=/usr
    message("No INSTALLBASE specified, defaulting to $$INSTALLBASE")
}
