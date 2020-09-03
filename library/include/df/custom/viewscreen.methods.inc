friend struct df::interfacest;
void feed_key(df::interface_key key) {
    std::set8<df::interface_key> input;
    input.insert(key);
    feed(&input);
}
