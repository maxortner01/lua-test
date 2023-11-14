Frame = 0
New_Entity = {}

function Start()
    Debug.log("hello")
end

function Update(world, entity)
    local position = entity:getComponent(Component.Position);
    Debug.assert(position.good)

    --Debug.log("Position = ("..position.x..", "..position.y..")")

    if Frame == 0 then
        New_Entity = world:createEntity({ type = Component.Position, value={ x = 200, y = 100 } })
        local pos = New_Entity:getComponent(Component.Position)
        pos.x = 900
        New_Entity:setComponent(pos)
    end

    if Frame == 500 then
        New_Entity:destroy()
        entity:destroy()
    end

    Debug.log(Frame)
    Frame = Frame + 1
end