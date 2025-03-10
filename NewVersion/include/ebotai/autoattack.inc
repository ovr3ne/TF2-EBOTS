stock void Attack(int client)
{
	if (m_class[client] == TFClass_DemoMan && IsWeaponSlotActive(client, 0) && TF2_GetPlayerWeaponID(client, 0) == 996)
	{
		if (!m_isSlowThink[client])
			AutoAttack(client);
	}
	else
	{
		if (m_hasEnemiesNear[client] && m_class[client] == TFClass_Spy)
		{
			SpyAttack(client);
			return;
		}
		
		if (m_hasEnemiesNear[client] && m_class[client] != TFClass_Spy)
		{
			AutoAttack(client);
			return;
		}
	}
}

stock void AutoAttack(int client)
{
	if (IsReloading(client))
		return;
	
	if (IsSniper(client))
	{
		if (IsWeaponSlotActive(client, 0))
		{
			if (m_isSlowThink[client] && TF2_IsPlayerInCondition(client, TFCond_Zoomed) && GetRandomInt(-1, m_difficulty[client]) >= 0)
				m_buttons[client] |= IN_ATTACK;

			if (m_difficulty[client] > 0 && m_lowHealth[client])
				return;

			if (!TF2_IsPlayerInCondition(client, TFCond_Zoomed))
				m_buttons[client] |= IN_ATTACK2;
			
			return;
		}
	}
	else if (m_primaryID[client] == 56 || m_primaryID[client] == 1005 || m_primaryID[client] == 1092 || m_primaryID[client] == 1098)
	{
		if (m_attackTimer[client] < GetGameTime())
			m_attackTimer[client] = GetGameTime() + GetRandomFloat(1.25, 2.25);
		return;
	}

	if (m_hasEnemiesNear[client] && m_class[client] == TFClass_Engineer)
	{
		m_attackTimer[client] = GetGameTime() + 0.4;
		return;
	}
	
	if (m_class[client] == TFClass_Heavy)
	{
		if (IsWeaponSlotActive(client, 0))
		{
			if (GetAmmo(client, 0) > 0)
			{
				if (m_lowHealth[client])
					m_attack2Timer[client] = GetGameTime() + 2.0;
				else
					m_attack2Timer[client] = GetGameTime() + 4.0;
				m_attackTimer[client] = GetGameTime() + 0.4;
			}
			else
			{
				m_attackTimer[client] = 0.0;
				m_attack2Timer[client] = 0.0;
			}
		}
		else
			m_attackTimer[client] = GetGameTime() + 0.4;
		return;
	}
	
	if (m_class[client] == TFClass_Pyro && IsWeaponSlotActive(client, 0))
	{
		m_attackTimer[client] = GetGameTime() + 0.4;
		return;
	}
	
	if (m_class[client] == TFClass_DemoMan)
	{
		if (IsWeaponSlotActive(client, 0) && m_primaryID[client] == 996)
		{
			if (m_attackTimer[client] + 0.125 < GetGameTime())
				m_attackTimer[client] = GetGameTime() + 0.125; // 8 stickies in 1 second
		}
		else
			m_attackTimer[client] = GetGameTime() + 0.4;
		return;
	}
	
	if (m_class[client] == TFClass_Soldier)
	{
		if (IsWeaponSlotActive(client, 0) && m_primaryID[client] == 730)
		{
			if (GetEntProp(GetPlayerWeaponSlot(client, 0), Prop_Send, "m_iClip1") < 3)
				m_attackTimer[client] = GetGameTime() + 0.4;
		}
		else if (IsWeaponSlotActive(client, 0) && m_primaryID[client] == 441)
		{
			if (GetEntProp(GetPlayerWeaponSlot(client, 0), Prop_Send, "m_iClip1") > 3 && GetRandomInt(1, 2) == 1)
				m_buttons[client] |= IN_ATTACK2;
			else
				m_attackTimer[client] = GetGameTime() + 0.4;
		}
		else
			m_attackTimer[client] = GetGameTime() + 0.4;
		return;
	}
	
	if (m_hasEnemiesNear[client] && !TF2_IsPlayerInCondition(m_nearestEnemy[client], TFCond_Ubercharged))
	{
		if (!TF2_IsPlayerInCondition(client, TFCond_Zoomed))
			m_attackTimer[client] = GetGameTime() + 0.4;
	}
	else
		m_attackTimer[client] = GetGameTime() + 0.45;
}

stock void SpyAttack(int client)
{
	if (IsReadyForAttack(client))
	{
		if (!IsWeaponSlotActive(client, 0))
		{
			float range = Squaredf(128.0);
			if (IsValidClient(m_nearestEnemy[client]) && GetVectorDistance(GetEyePosition(client), GetEyePosition(m_nearestEnemy[client]), true) > range)
				EquipWeaponSlot(client, 0);
			else if (m_enemyDistance[client] > range)
				EquipWeaponSlot(client, 0);
			else
				m_attackTimer[client] = GetGameTime() + 0.4;
		}
		else
			m_attackTimer[client] = GetGameTime() + 0.4;

		return;
	}
	
	if (m_hasEnemiesNear[client] && ChanceOf(m_eBotSenseChance[client]) && TF2_IsPlayerInCondition(m_nearestEnemy[client], TFCond_Taunting) && m_hasFriendsNear[client])
	{
		if (!IsWeaponSlotActive(client, 0))
		{
			float range = Squaredf(128.0);
			if (GetVectorDistance(GetEyePosition(client), GetEyePosition(m_nearestEnemy[client]), true) > range)
				EquipWeaponSlot(client, 0);
			else
				m_attackTimer[client] = GetGameTime() + 0.4;
		}
		else
			m_attackTimer[client] = GetGameTime() + 0.4;

		return;
	}
	
	if (m_hasEnemiesNear[client] && !ChanceOf(m_eBotSenseChance[client]) && (TF2_IsPlayerInCondition(m_nearestEnemy[client], TFCond_Disguised) || GetClientHealth(m_nearestEnemy[client]) <= 65.0))
	{
		if (!IsWeaponSlotActive(client, 0))
		{
			float range = Squaredf(128.0);
			if (GetVectorDistance(GetEyePosition(client), GetEyePosition(m_nearestEnemy[client]), true) > range)
				EquipWeaponSlot(client, 0);
			else
				m_attackTimer[client] = GetGameTime() + 0.4;
		}
		else
			m_attackTimer[client] = GetGameTime() + 0.4;

		return;
	}
}

stock bool IsReadyForAttack(int client) // 1 core cpu killer... 
{
	if (TF2_IsPlayerInCondition(client, TFCond_Ubercharged) 
	|| TF2_IsPlayerInCondition(client, TFCond_TmpDamageBonus) 
	|| TF2_IsPlayerInCondition(client, TFCond_Buffed) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritOnFirstBlood) 
	|| TF2_IsPlayerInCondition(client, TFCond_OnFire) 
	|| TF2_IsPlayerInCondition(client, TFCond_Jarated) 
	|| TF2_IsPlayerInCondition(client, TFCond_Bleeding) 
	|| TF2_IsPlayerInCondition(client, TFCond_Milked) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritCanteen) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritOnWin) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritOnFlagCapture) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritOnKill) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritOnDamage) 
	|| TF2_IsPlayerInCondition(client, TFCond_MiniCritOnKill) 
	|| TF2_IsPlayerInCondition(client, TFCond_CritRuneTemp) 
	|| TF2_IsPlayerInCondition(client, TFCond_Gas) 
	|| TF2_IsPlayerInCondition(client, TFCond_ObscuredSmoke) 
	|| TF2_IsPlayerInCondition(client, TFCond_HalloweenGiant) 
	|| TF2_IsPlayerInCondition(client, TFCond_UberchargedOnTakeDamage) 
	|| TF2_IsPlayerInCondition(client, TFCond_UberchargedHidden) 
	|| TF2_IsPlayerInCondition(client, TFCond_UberchargedCanteen) 
	|| TF2_IsPlayerInCondition(client, TFCond_HalloweenBombHead) 
	|| TF2_HasTheFlag(m_nearestEnemy[client]))
		return true;
	return false;
}

stock void SelectBestCombatWeapon(int client)
{
	if (!m_isSlowThink[client])
		return;
		
	float distance = GetVectorDistance(GetOrigin(client), m_lookAt[client], true);
	float meleerange = Squaredf(GetConVarFloat(EBotMelee));
	
	if (m_class[client] == TFClass_Pyro)
		meleerange *= 0.75;

	if (TF2_IsPlayerInCondition(client, TFCond_Disguised))
		meleerange = 300.0;
		
	if (distance <= meleerange)
		EquipWeaponSlot(client, 2);
	else
	{
		if (IsSniper(client))
		{
			if (m_secondaryID[client] == 57 || m_secondaryID[client] == 231 || m_secondaryID[client] == 642)
				EquipWeaponSlot(client, 0);
			else if (((m_secondaryID[client] == 58 || m_secondaryID[client] == 1083 || m_secondaryID[client] == 1105) && distance <= Squaredf(512.0)) && (IsValidEntity(GetPlayerWeaponSlot(client, 1)) && GetEntProp(GetPlayerWeaponSlot(client, 1), Prop_Send, "m_iPrimaryAmmoType") == 1))
				EquipWeaponSlot(client, 1);
			else if (distance <= Squaredf(512.0))
				EquipWeaponSlot(client, 1);
			else
				EquipWeaponSlot(client, 0);
		}
		else if (m_primaryID[client] == 56 || m_primaryID[client] == 1005 || m_primaryID[client] == 1092 || m_primaryID[client] == 1098)
		{
			if (m_attackTimer[client] < GetGameTime() && distance <= Squaredf(512.0))
				EquipWeaponSlot(client, 1);
			else
				EquipWeaponSlot(client, 0);
		}
		else if (m_class[client] == TFClass_Heavy)
		{
			if (m_secondaryID[client] == 311 || m_secondaryID[client] == 42 || m_secondaryID[client] == 863 || m_secondaryID[client] == 1002 || m_secondaryID[client] == 159 || m_secondaryID[client] == 433 || m_secondaryID[client] == 1190)
				EquipWeaponSlot(client, 0);				   
			else if (distance <= Squaredf(256.0) || (m_currentIndex[client] != -1 && m_paths[m_currentIndex[client]].flags == WAYPOINT_JUMP))
				EquipWeaponSlot(client, 1);
			else
				EquipWeaponSlot(client, 0);
		}
		else if (m_class[client] == TFClass_Pyro)
		{
			if (m_secondaryID[client] == 1180 && IsValidEntity(GetPlayerWeaponSlot(client, 1)) && GetEntProp(GetPlayerWeaponSlot(client, 1), Prop_Send, "m_iPrimaryAmmoType") == 1)
				EquipWeaponSlot(client, 1);
			else if (!(GetEntityFlags(client) & FL_FLY) && distance <= Squaredf(350.0))
				EquipWeaponSlot(client, 0);
			else
				EquipWeaponSlot(client, 1);
		}
		else if (m_class[client] == TFClass_Soldier)
		{
			if (m_secondaryID[client] == 129 || m_secondaryID[client] == 1001 || m_secondaryID[client] == 226 || m_secondaryID[client] == 354 || m_secondaryID[client] == 444)
				EquipWeaponSlot(client, 0);				   
			else if (distance <= Squaredf(256.0))
				EquipWeaponSlot(client, 1);
			else if (IsValidClient(m_nearestEnemy[client]) && m_class[m_nearestEnemy[client]] == TFClass_Pyro && distance <= Squaredf(512.0))
				EquipWeaponSlot(client, 1);
			else
				EquipWeaponSlot(client, 0);
		}
		else if (m_class[client] == TFClass_Medic)
			EquipWeaponSlot(client, 0);
		else if (m_class[client] == TFClass_DemoMan)
			EquipWeaponSlot(client, 0);
		else if (m_class[client] == TFClass_Scout)
		{
			if ((m_meleeID[client] == 648 || m_meleeID[client] == 44) && IsValidEntity(GetPlayerWeaponSlot(client, 1)) && GetEntProp(GetPlayerWeaponSlot(client, 1), Prop_Send, "m_iPrimaryAmmoType") == 1)
				EquipWeaponSlot(client, 2);
			else if ((m_secondaryID[client] == 812 || m_secondaryID[client] == 833 || m_secondaryID[client] == 1121 || m_secondaryID[client] == 222) && IsValidEntity(GetPlayerWeaponSlot(client, 1)) && GetEntProp(GetPlayerWeaponSlot(client, 1), Prop_Send, "m_iPrimaryAmmoType") == 1)
				EquipWeaponSlot(client, 1);
			else if (m_secondaryID[client] == 46 || m_secondaryID[client] == 1145 || m_secondaryID[client] == 163)
				EquipWeaponSlot(client, 0);
			else if (distance >= Squaredf(1024.0))
				EquipWeaponSlot(client, 1);
			else
				EquipWeaponSlot(client, 0);
		}
		else if (m_class[client] == TFClass_Engineer)
		{
			if (m_secondaryID[client] == 528)
				EquipWeaponSlot(client, 0);
			else if (distance >= Squaredf(1024.0))
				EquipWeaponSlot(client, 1);
			else
				EquipWeaponSlot(client, 0);
		}
		else if (m_class[client] == TFClass_Spy)
			EquipWeaponSlot(client, 0);

		if (IsWeaponSlotActive(client, 2) && distance > meleerange)
		{
			int clip0 = GetPrimaryClip(client, 0);
			int clip1 = GetPrimaryClip(client, 1);

			if (m_class[client] == TFClass_Medic)
				EquipWeaponSlot(client, 0);
			else if (m_class[client] == TFClass_Pyro || m_class[client] == TFClass_Heavy)
			{
				if (clip1 <= 0)
					EquipWeaponSlot(client, 1);
				else
					EquipWeaponSlot(client, 0);
			}
			else if (clip0 <= 0)
				EquipWeaponSlot(client, 0);
			else if (clip1 <= 0)
				EquipWeaponSlot(client, 1);
			else
				EquipWeaponSlot(client, 0);
		}
	}
}
