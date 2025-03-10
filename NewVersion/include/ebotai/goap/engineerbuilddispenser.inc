public bool BuildDispenserRequirements(int client)
{
	if (!IsValidEntity(SentryGun[client]))
		return false;
		
	if (!SentryHealthIsFull[client])
		return false;

	if (GetMetal(client) < 100.0)
		return false;

	if (IsMoving(client))
		return false;

	return true;
}

public void BuildDispenserStart(int client)
{
	// :(
}

public void BuildDispenserUpdate(int client)
{
	CheckHealth(client);
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	LookUpdate(client);

	if (!IsWeaponSlotActive(client, 5))
		FakeClientCommandThrottled(client, "build 0 0");
	else
	{
		if (IsValidEntity(SentryGun[client]))
		{
			float randomArea[3];
			randomArea = GetCenter(SentryGun[client]);
			randomArea[0] += GetRandomFloat(-600.0, 600.0);
			randomArea[1] += GetRandomFloat(-600.0, 600.0);
			FollowPath(client, randomArea);

			if (IsVisible(GetEyePosition(client), randomArea) || IsVisible(GetEyePosition(client), GetCenter(SentryGun[client])))
			{
				m_ignoreEnemies[client] = GetGameTime() + 1.0;
				m_buttons[client] |= IN_ATTACK;
			}
		}
		else
		{
			m_buttons[client] |= IN_ATTACK;
			FinishCurrentProcess(client);
		}
	}
	
	int dispenser = TF2_GetObject(client, TFObject_Dispenser, TFObjectMode_None);
	if (IsValidEntity(dispenser))
	{
		FinishCurrentProcess(client);
		EngineerMoveDispenserTimer[client] = GetGameTime() + GetRandomFloat(120.0, 180.0);
		WantsBuildDispenser[client] = false;
		WantsMoveDispenser[client] = false;
	}
}

public void BuildDispenserEnd(int client)
{
	EquipWeaponSlot(client, 0);
}