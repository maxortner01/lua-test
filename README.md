# Simple 2D Engine
I am a chronic "lowest-leveler" and I love writing graphics engines. But I've also wanted to make a game for a long time. These things don't really mix, as any time I get a game idea I inevitably end up writing a rendering engine and move on before I ever get to the game stage (check my repo list for proof).

However, I want to construct a basic 2D game engine that is light-weight and utilizes some technology already out there to get the job done. I will probably end up writing a rendering engine again, but not first!

## How it works
It is very much WIP, in fact I haven't written the code to compile in any other environment except my own. Because of this, it is laden with dependencies. It expects `lua`, `libccd`, and `eigen3` to be installed somewhere on the machine aside from the dependencies that come as submodules.

It **does not** attempt to abstract away the dependencies or hide them, it is simple attempting to "gather" the technology together in a coherent and simple to use form in order to make games. I will probably change this in the future, but I'm simply getting a proof-of-concept at the moment.

## ECS
It uses `flecs` for the entity-component-system backend which is what drives most of the functionality.

There is no way to write components in C++ and import them *however*, there is functionality for scripted components (more on that later). These are the currently supported components in the `S2D::Engine` namespace.

### Components

#### Sprite
A simple sprite: A quad with a texture. The mesh for the quad will eventually be instanced from a single generated quad mesh.

```C++
struct Sprite
{
    // Contains a mesh (handled by S2D)
    Lua::String texture;
    sf::Vector2u size;
};
```

The Lua table representation is
```Lua
sprite = {

};
```

#### Tilemap
Represents a "sparse" grid of tiles. It is stored as a hash-map of 2D `int16_t` position keys pointing to tile information (2D tile index in texture).

```C++
struct Tilemap
{
    // Contains a mesh (handled by S2D)
    // Contains a map of tiles (handled by S2D)
    sf::Vector2f tilesize;
    struct 
    {
        Lua::String texture_name;
    } spritesheet;
};
```

The Lua table representation is
```Lua
tilemap = {
    tilesize = { width = [Number], height = [Number] },
    spritesheet = { textureName = [String] }
}
```
which has two functions contained within it
```Lua
tilemap:setLayerState(
    1,               -- Layer to adjust
    LayerState.Solid -- State to set
)

tilemap:setTile(
    0,     -- Layer to place tile in
    x, y,  -- Coordinates in tilemap to place
    cx, cy -- Coordinates from texture to set tile
)
```

There are two types of states `LayerState.Solid` and `LayerState.NotSolid`, the default is the latter. This specification is irrelevant unless the entity that has a tilemap also has a `Collider` component that specifies the tilemap as its source, in which case the `LayerState` for each layer is used to generate the correct collision mesh.