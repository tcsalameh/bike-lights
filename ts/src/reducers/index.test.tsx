import { incrementEnthusiasm, decrementEnthusiasm } from '../actions';
import { StoreState } from '../types/index';
import { enthusiasm } from './index';

it('returns state w/ enthusiasm incremented by one when called with INCREMENT_ENTHUSIASM', () => {
  const initState: StoreState = { languageName: 'typescript', enthusiasmLevel: 1 };
  const endState: StoreState = { languageName: 'typescript', enthusiasmLevel: 2 };
  expect(enthusiasm(initState, incrementEnthusiasm())).toEqual(endState);
});

it('returns state w/ enthusiasm decremented by one when called with DECREMENT_ENTHUSIASM', () => {
  const initState: StoreState = { languageName: 'typescript', enthusiasmLevel: 2 };
  const endState: StoreState = { languageName: 'typescript', enthusiasmLevel: 1 };
  expect(enthusiasm(initState, decrementEnthusiasm())).toEqual(endState);
});

it('stops at a minimum enthusiasm of 1', () => {
  const initState: StoreState = { languageName: 'typescript', enthusiasmLevel: 1 };
  const endState: StoreState = { languageName: 'typescript', enthusiasmLevel: 1 };
  expect(enthusiasm(initState, decrementEnthusiasm())).toEqual(endState);
});


