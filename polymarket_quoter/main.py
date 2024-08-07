from py_clob_client.client import ClobClient
from py_clob_client.clob_types import ApiCreds, FilterParams, DropNotificationParams

from py_clob_client.constants import POLYGON
from pprint import pprint

import pandas as pd
import time


def get_all_simplified_markets(client):
    resp = client.get_markets()
    data = resp['data']
    next_cursor = resp['next_cursor']

    while (next_cursor != "LTE="):
        print(f'next cursor={next_cursor}')
        time.sleep(0.5)
        resp = client.get_markets(next_cursor)
        next_cursor = resp['next_cursor']
        data.extend(resp['data'])
    return data

def main():

    # Add private API key
    key = ""
    with open('private.txt', 'r') as file:
        key = file.read().rstrip()

    host = "https://clob.polymarket.com"
    chain_id = POLYGON
    client = ClobClient(host, key=key, chain_id=chain_id)
    # create API key, or use existing key
    client.set_api_creds(client.create_or_derive_api_creds())

    if (not client.get_ok()):
        return

    # market data import
    print(f'Server time is {client.get_server_time()}')
    data = get_all_simplified_markets(client)

    # data cleaning
    df = pd.DataFrame(data)
    df['rewards_daily_rate'] = df['rewards'].apply(lambda x: x['rates'][0].get('rewards_daily_rate', 0) if x['rates'] is not None else 0)
    df_light = df[['rewards_daily_rate', 'closed', 'active', 'condition_id', 'description', 'question']]

    #Daily Rewards Total Calculation

    # Step 1: Count the occurrences of each value
    open_df = df[(df['closed'] == False) & (df['active'] == True)]
    counts = open_df['rewards_daily_rate'].value_counts()

    # Step 2: Multiply the counts by the values
    result = counts * counts.index

    # Display the result
    print(result)

    print(f"total daily rewards: {result.sum()}")

    # resp = client.get_orders()
    # pprint(resp)
    # pprint(client.drop_notifications(DropNotificationParams(ids=["925581"])))
    # pprint(client.get_notifications())
    # resp = client.get_trades(
    #         FilterParams(
    #             limit=1000,
    #             taker=client.get_address(),
    #             maker=client.get_address(),
    #             market="0x3353f0005d273aa63e233aa107e71cf8d95744c6209e8e0c857d6555768190f2"
    #             )
    #         )
    # pprint(resp)

main()
