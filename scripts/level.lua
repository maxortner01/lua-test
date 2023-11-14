-- Idea of how to construct levels from a lua script: The next major lua thing
Entities = {
    {
        {
            type = Component.Position,
            value = {
                x = 10,
                y = 10
            }
        },
        {
            type = Component.Script,
            value = { "lua" }
        }
    }
}