


SelectedSpellId = 0;


function DoAbility(mouse_pos, target_pos)

    local new_effect = createEffect("spell4", "Effects/consecration.lua");
    new_effect.pos = mouse_pos;
    new_effect.lifetime = 5.;
    setPosition("spell4", mouse_pos.x, mouse_pos.y);

    print(new_effect.pos.x, new_effect.pos.y);
    local wtf = getObject("spell4");
    print("XXX ", wtf.pos.x, wtf.pos.y);
end
    

