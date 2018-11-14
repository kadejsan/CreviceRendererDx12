#pragma once

inline void ThrowIfFailed( HRESULT hr )
{
	if( FAILED( hr ) )
	{
		throw std::exception();
	}
}