bool m_teleExitTriggerBlock[TFMaxPlayers];

public bool BuildTeleExitRequirements(int client)
{
	if (m_teleExitTriggerBlock[client])
		return false;
	
	if (m_exitFail[client])
		return false;

	if (GetMetal(client) < 50.0)
		return false;

	return true;
}

float teleexitbuildarea[3];
public void BuildTeleExitStart(int client)
{
	PickTeleExitSpot(client, teleexitbuildarea);
	if (!IsNullVector(teleexitbuildarea))
		DeletePathNodes(client);
	else
	{
		m_teleExitTriggerBlock[client] = true;
		FinishCurrentProcess(client);
	}
}

public void BuildTeleExitUpdate(int client)
{
	CheckHealth(client);
	FindFriendsAndEnemiens(client);
	FindEnemyEntities(client);
	
	float range = GetVectorDistance(GetOrigin(client), teleexitbuildarea, true);
	if (range <= Squaredf(300.0))
	{
		m_ignoreEnemies[client] = GetGameTime() + 1.0;

		if (!IsWeaponSlotActive(client, 5) && !IsMoving(client))
			FakeClientCommandThrottled(client, "build 1 1");
			
		if ((m_hasWaypoints || m_hasNavpoints) && m_isSlowThink[client])
		{
			int index = m_goalIndex[client];
			if (index != -1)
			{
				if (GetRandomInt(1, 2) == 1)
					m_lookAt[client] = m_paths[index].campStart;
				else
					m_lookAt[client] = m_paths[index].campEnd;
			}
		}

		LookAtPosition(client, m_lookAt[client], GetEyeAngles(client));

		if (IsWeaponSlotActive(client, 5))
		{
			if (!IsMoving(client))
				m_buttons[client] |= IN_ATTACK;
			
			int objBeingBuilt = GetEntPropEnt(GetActiveWeapon(client), Prop_Send, "m_hObjectBeingBuilt");
			if (!IsValidEntity(objBeingBuilt))
				return;
			
			if (m_isSlowThink[client] && GetRandomInt(1, 10) == 1)
				PickTeleExitSpot(client, teleexitbuildarea);
			
			return;
		}
	}
	else
		LookUpdate(client);

	if (range > Squaredf(70.0))
		FollowPath(client, teleexitbuildarea);
	else
		MoveToWaypoint(client, teleexitbuildarea);
	
	int teleexit = TF2_GetObject(client, TFObject_Teleporter, TFObjectMode_Exit);
	if (IsValidEntity(teleexit))
	{
		FinishCurrentProcess(client);
		WantsBuildTeleporterExit[client] = false;
	}
}

public void BuildTeleExitEnd(int client)
{
	m_exitFail[client] = true;
	EquipWeaponSlot(client, 0);
}

stock void PickTeleExitSpot(int client, float area[3] = NULL_VECTOR)
{
	if ((m_hasWaypoints || m_hasNavpoints) && m_hasTeleporterExitWaypoints)
	{
		int index = -1;
		ArrayList GoodTeleExitSpots = new ArrayList();
		ArrayList AllTeleExitSpots = new ArrayList();
		for (int i = 0; i < m_waypointNumber; i++)
		{
			if (!(m_paths[i].flags & WAYPOINT_TELEPORTEREXIT))
				continue;
			
			// blocked waypoint
   			if (m_paths[i].activeArea != 0 && !(m_paths[i].activeArea & currentActiveArea))
				continue;
			
			if (m_lastFailedWaypoint[client] == i)
				continue;
			
    		// not for our team
 			if (m_team[client] == 3 && m_paths[i].team == 2)
    			continue;
   			else if (m_team[client] == 2 && m_paths[i].team == 3)
        		continue;

			if (!IsWaypointUsed(i))
				GoodTeleExitSpots.Push(i);
			else
				AllTeleExitSpots.Push(i);
		}

		if (index == -1)
		{
			if (GoodTeleExitSpots.Length > 0)
				index = GoodTeleExitSpots.Get(GetRandomInt(0, GoodTeleExitSpots.Length - 1));
			else if (AllTeleExitSpots.Length > 0)
				index = AllTeleExitSpots.Get(GetRandomInt(0, AllTeleExitSpots.Length - 1));
		}

		delete GoodTeleExitSpots;
		delete AllTeleExitSpots;

		if (index != -1)
		{
			m_goalIndex[client] = index;
			if (m_paths[index].radius > 0)
			{
				area[0] = m_paths[index].origin[0] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
				area[1] = m_paths[index].origin[1] + GetRandomFloat(-m_paths[index].radius, m_paths[index].radius);
				area[2] = m_paths[index].origin[2];
			}
			else
				area = m_paths[index].origin;
		}
	}
}