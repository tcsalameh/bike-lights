import * as React from 'react';
import * as enzyme from 'enzyme';
import RGBSelector from './rgb';

it('renders anything at all', () => {
  const thingy = enzyme.shallow(<div>
      <h1>Test</h1>
    </div>);
  expect(thingy.find('h1').html()).toMatch(/Test/);
});

const wrapper = enzyme.shallow(<RGBSelector />);
let container: any, containerProp: any;

describe("rgb selector - basic", () => {
  beforeEach(() => {
    container = wrapper.find("input");
    containerProp = container.props();
  });

  it('should have an input field', () => {
    expect(container).toHaveLength(1);
  });

  it('input field has classname rgb-input', () => {
    expect(containerProp.className).toEqual("rgb-input");
  });

  it('input field contains no text to start', () => {
    expect(container.text()).toEqual("");
  });
});

  
